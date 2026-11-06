#include "bme280.h"
#include <stddef.h>
#include <string.h>

/* Internal helpers */
static uint16_t BME280_U16LE(const uint8_t *p);
static int16_t BME280_S16LE(const uint8_t *p);
static int16_t BME280_S12(uint16_t value);
static int BME280_ConfigIsValid(const BME280_Config *cfg);
static BME280_Status BME280_ReadCalibration(BME280_Handle *dev);
static BME280_Status BME280_Configure(BME280_Handle *dev);
static BME280_Status BME280_WaitStatusClear(BME280_Handle *dev, uint8_t mask);
static BME280_Status BME280_ReadRegisters(BME280_Handle *dev, uint8_t reg, uint8_t *data, uint8_t length);
static BME280_Status BME280_ReadRegister(BME280_Handle *dev, uint8_t reg, uint8_t *value);
static BME280_Status BME280_WriteRegister(BME280_Handle *dev,uint8_t reg,uint8_t value);
static BME280_Status BME280_Compensate(BME280_Handle *dev, const BME280_RawData *raw, BME280_Data *data);

/* Combining 2 bytes in one unsigned int, in the Little Endian format */
static uint16_t BME280_U16LE(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

/* Combining 2 bytes in one signed int, in the Little Endian format */
static int16_t BME280_S16LE(const uint8_t *p)
{
    uint16_t value = BME280_U16LE(p);
    int32_t signed_value;

    if (value & 0x8000U) signed_value = (int32_t)value-65536;
    else signed_value = (int32_t)value;

    return (int16_t)signed_value;
}

/* Convert a signed 12-bit two's-complement value to int16_t. */
static int16_t BME280_S12(uint16_t value)
{
    value &= 0x0FFFU;
    if (value & 0x0800U) return (int16_t)((int32_t)value-4096);
    return (int16_t)value;
}

static int BME280_ConfigIsValid(const BME280_Config *cfg)
{
    if (cfg == NULL) return 0;

    if ((unsigned)cfg->temperature_oversampling > 5U || (unsigned)cfg->pressure_oversampling > 5U ||
        (unsigned)cfg->humidity_oversampling > 5U)
        return 0;

    if ((unsigned)cfg->filter > 4U)
        return 0;

    if ((unsigned)cfg->standby > 7U)
        return 0;

    if (cfg->mode != BME280_MODE_SLEEP && cfg->mode != BME280_MODE_NORMAL 
        && cfg->mode != BME280_MODE_FORCED)
        return 0;

    return 1;
}

/* Default configuration */

void BME280_GetDefaultConfig(BME280_Config *config)
{
    if (config == NULL) return;

    config->temperature_oversampling = BME280_OVERSAMPLING_1X;
    config->pressure_oversampling = BME280_OVERSAMPLING_4X;
    config->humidity_oversampling = BME280_OVERSAMPLING_1X;

    config->filter = BME280_FILTER_4;
    config->standby = BME280_STANDBY_500_MS;
    config->mode = BME280_MODE_NORMAL;
}

/* Register access */
static BME280_Status BME280_ReadRegisters(BME280_Handle *dev, uint8_t reg, uint8_t *data, uint8_t length)
{
    if (dev == NULL || data == NULL || length == 0U)
    {
        return BME280_ERROR_INVALID_PARAM;
    }
    I2C_Status status = I2C_WriteRead(&dev->hi2c, dev->address, &reg, 1U, data, length);
    if (status != I2C_OK)
    {
        return BME280_ERROR_BUS;
    }
    return BME280_OK;
}

static BME280_Status BME280_ReadRegister(BME280_Handle *dev, uint8_t reg, uint8_t *value)
{
    return BME280_ReadRegisters(dev, reg, value, 1U);
}

static BME280_Status BME280_WriteRegister(BME280_Handle *dev,uint8_t reg,uint8_t value)
{
    if (dev == NULL)
    {
        return BME280_ERROR_INVALID_PARAM;
    }
    uint8_t buffer[2] = {reg, value};
    I2C_Status status = I2C_Write(&dev->hi2c, dev->address,buffer,2U);

    if (status != I2C_OK)
    {
        return BME280_ERROR_BUS;
    }
    return BME280_OK;
}

/* Wait for a status bit to clear */
static BME280_Status BME280_WaitStatusClear(BME280_Handle *dev, uint8_t mask)
{
    uint32_t timeout;
    uint8_t status;
    BME280_Status result;

    for (timeout = 0U; timeout < BME280_TIMEOUT; timeout++)
    {
        result = BME280_ReadRegister(dev, BME280_REG_STATUS, &status);

        if (result != BME280_OK)
            return result;

        if ((status & mask) == 0U)
            return BME280_OK;
    }

    return BME280_ERROR_TIMEOUT;
}

/* Read factory calibration coefficients */
static BME280_Status BME280_ReadCalibration(BME280_Handle *dev)
{
    uint8_t tp[BME280_CALIB_TP_LENGTH];
    uint8_t h[BME280_CALIB_H_LENGTH];
    BME280_Calibration *c = &dev->calib;
    BME280_Status result;

    result = BME280_ReadRegisters(dev, BME280_REG_CALIB_TP, tp, sizeof(tp));
    if (result != BME280_OK) return result;

    result = BME280_ReadRegisters(dev, BME280_REG_CALIB_H, h, sizeof(h));
    if (result != BME280_OK) return result;

    /* Temperature coefficients: 0x88..0x8D */
    c->dig_T1 = BME280_U16LE(&tp[0]);
    c->dig_T2 = BME280_S16LE(&tp[2]);
    c->dig_T3 = BME280_S16LE(&tp[4]);

    /* Pressure coefficients: 0x8E..0x9F */
    c->dig_P1 = BME280_U16LE(&tp[6]);
    c->dig_P2 = BME280_S16LE(&tp[8]);
    c->dig_P3 = BME280_S16LE(&tp[10]);
    c->dig_P4 = BME280_S16LE(&tp[12]);
    c->dig_P5 = BME280_S16LE(&tp[14]);
    c->dig_P6 = BME280_S16LE(&tp[16]);
    c->dig_P7 = BME280_S16LE(&tp[18]);
    c->dig_P8 = BME280_S16LE(&tp[20]);
    c->dig_P9 = BME280_S16LE(&tp[22]);

    /* H1 is stored separately at address 0xA1. */
    c->dig_H1 = tp[25];

    /*
        Humidity coefficients:
        H2: E1/E2
        H3: E3
        H4: E4 and E5[3:0]
        H5: E5[7:4] and E6
        H6: E7
    */
    c->dig_H2 = BME280_S16LE(&h[0]);
    c->dig_H3 = h[2];

    c->dig_H4 = BME280_S12(((uint16_t)h[3] << 4) | ((uint16_t)h[4] & 0x0FU));
    c->dig_H5 = BME280_S12( ((uint16_t)h[5] << 4) | ((uint16_t)h[4] >> 4));

    /* Preserve the signed 8-bit representation portably. */
    c->dig_H6 = (h[6] < 128U) ? (int8_t)h[6] : (int8_t)((int16_t)h[6] - 256);

    /* dig_P1 must be nonzero for pressure compensation. Reject an invalid calibration block. */
    if (c->dig_T1 == 0U || c->dig_P1 == 0U) return BME280_ERROR_CALIBRATION;

    return BME280_OK;
}

/* Configure oversampling, filter and operating mode */

static BME280_Status BME280_Configure(BME280_Handle *dev)
{
    uint8_t value;
    BME280_Status result;
    const BME280_Config *cfg = &dev->config;

    /* Ensure sleep mode before configuring config (0xF5) */
    result = BME280_ReadRegister(dev, BME280_REG_CTRL_MEAS, &value);

    if (result != BME280_OK) return result;

    value &= (uint8_t)~0x03U;

    result = BME280_WriteRegister(dev, BME280_REG_CTRL_MEAS, value);
    if (result != BME280_OK) return result;

    /* Humidity oversampling. The setting becomes effective after ctrl_meas is written */
    value = (uint8_t)cfg->humidity_oversampling;

    result = BME280_WriteRegister(dev, BME280_REG_CTRL_HUM, value);

    if (result != BME280_OK)
        return result;

    /*
        config:
        [7:5] standby
        [4:2] filter
        [1]   reserved
        [0]   SPI 3-wire enable
        SPI 3-wire remains disabled.
    */
    value = (uint8_t)(((uint8_t)cfg->standby << 5) | ((uint8_t)cfg->filter << 2));

    result = BME280_WriteRegister(dev, BME280_REG_CONFIG, value);

    if (result != BME280_OK)
        return result;

    /*
        ctrl_meas:
        [7:5] temperature oversampling
        [4:2] pressure oversampling
        [1:0] operating mode
    */
    value = (uint8_t)(((uint8_t)cfg->temperature_oversampling << 5) | ((uint8_t)cfg->pressure_oversampling << 2) |
        (uint8_t)cfg->mode);

    result = BME280_WriteRegister(dev, BME280_REG_CTRL_MEAS, value);

    if (result != BME280_OK)
        return result;

    return BME280_OK;
}

/* Initialization */

BME280_Status BME280_Init(BME280_Handle *dev, const I2C_Handle *hi2c, uint8_t address, const BME280_Config *config)
{
    uint8_t chip_id;
    BME280_Status result;
    BME280_Config default_config;

    if (dev == NULL || hi2c == NULL || hi2c->instance == NULL) return BME280_ERROR_INVALID_PARAM;
    if (address != BME280_I2C_ADDR_PRIMARY && address != BME280_I2C_ADDR_SECONDARY)
        return BME280_ERROR_INVALID_PARAM;

    if (config != NULL)
    {
        if (!BME280_ConfigIsValid(config)) return BME280_ERROR_INVALID_PARAM;
    }
    else
    {
        BME280_GetDefaultConfig(&default_config);
        config = &default_config;
    }

    /* Initialize the handle before the first bus transaction */
    memset(dev, 0, sizeof(*dev));

    dev->hi2c = *hi2c;
    dev->address= address;
    dev->config= *config;

    /* Verify sensor identity. */
    result = BME280_ReadRegister(dev, BME280_REG_CHIP_ID, &chip_id);

    if (result != BME280_OK)
        return result;

    if (chip_id != BME280_CHIP_ID)
        return BME280_ERROR_ID;

    /* Soft reset: writing 0xB6 initiates a power-on reset. */
    result = BME280_WriteRegister(dev, BME280_REG_RESET, BME280_RESET_VALUE);

    if (result != BME280_OK)
        return result;

    /* Wait until the NVM calibration copy is complete. */
    result = BME280_WaitStatusClear(dev, BME280_STATUS_IM_UPDATE);

    if (result != BME280_OK)
        return result;

    /* Read calibration only after reset and NVM update */
    result = BME280_ReadCalibration(dev);

    if (result != BME280_OK)
        return result;

    result = BME280_Configure(dev);

    if (result != BME280_OK)
        return result;

    dev->initialized = 1U;

    return BME280_OK;
}

/* Read raw measurements using one burst transaction */

BME280_Status BME280_ReadRawData(BME280_Handle *dev, BME280_RawData *raw)
{
    uint8_t buf[BME280_RAW_DATA_LENGTH];
    BME280_Status result;

    if (dev == NULL || raw == NULL) return BME280_ERROR_INVALID_PARAM;
    if (dev->initialized == 0U) return BME280_ERROR;

    /* One burst read from 0xF7 through 0xFE:
        F7 F8 F9: pressure
        FA FB FC: temperature
        FD FE:    humidity
    */
    result = BME280_ReadRegisters(dev, BME280_REG_PRESS_MSB, buf, sizeof(buf));
    if (result != BME280_OK) return result;

    raw->pressure = ((uint32_t)buf[0] << 12) | ((uint32_t)buf[1] << 4)  | ((uint32_t)buf[2] >> 4);
    raw->temperature = ((uint32_t)buf[3] << 12) | ((uint32_t)buf[4] << 4)  | ((uint32_t)buf[5] >> 4);
    raw->humidity =((uint16_t)buf[6] << 8) | (uint16_t)buf[7];

    return BME280_OK;
}

/* Compensation formulas */

static BME280_Status BME280_Compensate(BME280_Handle *dev, const BME280_RawData *raw, BME280_Data *data)
{
    const BME280_Calibration *c;
    double var1, var2, var3;
    double temperature;
    double pressure;
    double humidity;
    double h;

    if (dev == NULL || raw == NULL || data == NULL) return BME280_ERROR_INVALID_PARAM;
    if (dev->initialized == 0U) return BME280_ERROR;

    c = &dev->calib;

    /* Temperature compensation. Result is in °c
        t_fine is required by pressure and humidity.
    */
    var1 = ((double)raw->temperature / 16384.0 - (double)c->dig_T1 / 1024.0) * (double)c->dig_T2;
    var2 = ((double)raw->temperature / 131072.0 - (double)c->dig_T1 / 8192.0);
    var2 = var2 * var2 * (double)c->dig_T3;

    dev->t_fine = var1 + var2;
    temperature = dev->t_fine / 5120.0;

    /* Pressure compensation. Result is in Pascals. */
    var1 = dev->t_fine / 2.0 - 64000.0;

    var2 = var1 * var1 * (double)c->dig_P6 / 32768.0;
    var2 += var1 * (double)c->dig_P5 * 2.0;
    var2 = var2 / 4.0 + (double)c->dig_P4 * 65536.0;

    var1 = ((double)c->dig_P3 * var1 * var1 / 524288.0 + (double)c->dig_P2 * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0) * (double)c->dig_P1;

    if (var1 == 0.0) return BME280_ERROR_CALIBRATION;

    pressure = 1048576.0 - (double)raw->pressure;
    pressure = (pressure - var2 / 4096.0) * 6250.0 / var1;

    var1 = (double)c->dig_P9 * pressure * pressure / 2147483648.0;

    var2 = pressure * (double)c->dig_P8 / 32768.0;

    var3 = pressure / 256.0;
    var3 = var3 * var3 * var3 * (double)c->dig_P7 / 131072.0;

    pressure += (var1 + var2 + var3) / 16.0;

    /* Humidity compensation. Result is relative humidity in percent */
    h = dev->t_fine - 76800.0;

    humidity = ((double)raw->humidity - ((double)c->dig_H4 * 64.0 + (double)c->dig_H5 / 16384.0 * h))
                * ((double)c->dig_H2 / 65536.0
                * (1.0 + (double)c->dig_H6 / 67108864.0 * h * 
                (1.0 + (double)c->dig_H3 / 67108864.0 * h)));

    humidity *= (1.0 - (double)c->dig_H1 * humidity / 524288.0);

    if (humidity < 0.0) humidity = 0.0;

    if (humidity > 100.0) humidity = 100.0;

    data->temperature_c = temperature;
    data->pressure_pa = pressure;
    data->pressure_hpa = pressure / 100.0;
    data->humidity_rh = humidity;

    return BME280_OK;
}

/* Read compensated measurements */

BME280_Status BME280_ReadMeasurements(BME280_Handle *dev, BME280_Data *data)
{
    BME280_RawData raw;
    BME280_Status result;

    if (dev == NULL || data == NULL) return BME280_ERROR_INVALID_PARAM;

    result = BME280_ReadRawData(dev, &raw);
    if (result != BME280_OK) return result;

    return BME280_Compensate(dev, &raw, data);
}

/* Forced measurement */

BME280_Status BME280_TriggerForcedMeasurement(BME280_Handle *dev)
{
    uint8_t value;
    BME280_Status result;

    if (dev == NULL) return BME280_ERROR_INVALID_PARAM;
    if (dev->initialized == 0U) return BME280_ERROR;

    result = BME280_ReadRegister(dev, BME280_REG_CTRL_MEAS, &value);
    if (result != BME280_OK) return result;

    /* Set mode bits to 01: one forced measurement. */
    value = (uint8_t)((value & (uint8_t)~0x03U) | 0x01U);
    result = BME280_WriteRegister(dev, BME280_REG_CTRL_MEAS, value);

    if (result != BME280_OK) return result;
    /* In forced mode, the sensor returns to sleep after the measurement. Wait for the measuring bit to clear */
    return BME280_WaitStatusClear(dev, BME280_STATUS_MEASURING);
}