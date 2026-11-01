#ifndef BME280_H
#define BME280_H

#include <stdint.h>


/* I2C 7-bit addresses */
#define BME280_I2C_ADDR_PRIMARY       0x76U
#define BME280_I2C_ADDR_SECONDARY     0x77U

/* Register map */
#define BME280_REG_CALIB_TP            0x88U
#define BME280_REG_CHIP_ID             0xD0U
#define BME280_REG_RESET               0xE0U
#define BME280_REG_CALIB_H             0xE1U
#define BME280_REG_CTRL_HUM            0xF2U
#define BME280_REG_STATUS              0xF3U
#define BME280_REG_CTRL_MEAS           0xF4U
#define BME280_REG_CONFIG              0xF5U
#define BME280_REG_PRESS_MSB           0xF7U

#define BME280_CHIP_ID                 0x60U
#define BME280_RESET_VALUE             0xB6U

#define BME280_STATUS_MEASURING        (1U << 3)
#define BME280_STATUS_IM_UPDATE        (1U << 0)

/* Driver limits */
#define BME280_TIMEOUT                 100000U
#define BME280_CALIB_TP_LENGTH         26U
#define BME280_CALIB_H_LENGTH          7U
#define BME280_RAW_DATA_LENGTH         8U

/* Return codes */
typedef enum
{
    BME280_OK = 0,
    BME280_ERROR,
    BME280_ERROR_ID,
    BME280_ERROR_TIMEOUT,
    BME280_ERROR_INVALID_PARAM,
    BME280_ERROR_CALIBRATION,
    BME280_ERROR_BUS
} BME280_Status;


/* Oversampling settings: register encoding */
typedef enum
{
    BME280_OVERSAMPLING_SKIPPED = 0,
    BME280_OVERSAMPLING_1X = 1,
    BME280_OVERSAMPLING_2X = 2,
    BME280_OVERSAMPLING_4X = 3,
    BME280_OVERSAMPLING_8X = 4,
    BME280_OVERSAMPLING_16X = 5

} BME280_Oversampling;

/* IIR filter settings: register encoding */
typedef enum
{
    BME280_FILTER_OFF = 0,
    BME280_FILTER_2 = 1,
    BME280_FILTER_4 = 2,
    BME280_FILTER_8 = 3,
    BME280_FILTER_16 = 4

} BME280_Filter;

/* Standby duration settings: register encoding */
typedef enum
{
    BME280_STANDBY_0_5_MS = 0,
    BME280_STANDBY_62_5_MS,
    BME280_STANDBY_125_MS,
    BME280_STANDBY_250_MS,
    BME280_STANDBY_500_MS,
    BME280_STANDBY_1000_MS,
    BME280_STANDBY_10_MS,
    BME280_STANDBY_20_MS

} BME280_Standby;

/* Operating mode */
typedef enum
{
    BME280_MODE_SLEEP  = 0,
    BME280_MODE_FORCED = 1,
    BME280_MODE_NORMAL = 3
} BME280_Mode;

/* Factory calibration coefficients */
typedef struct
{
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;

    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;
} BME280_Calibration;

/* Raw, uncompensated measurements */
typedef struct
{
    uint32_t pressure;
    uint32_t temperature;
    uint16_t humidity;
} BME280_RawData;

/* Compensated measurements in physical units */
typedef struct
{
    double temperature_c;       /* degrees Celsius */
    double pressure_pa;         /* Pascals */
    double pressure_hpa;        /* hectopascals */
    double humidity_rh;         /* relative humidity, percent */

} BME280_Data;

/* User-selectable sensor configuration */
typedef struct
{
    BME280_Oversampling temperature_oversampling;
    BME280_Oversampling pressure_oversampling;
    BME280_Oversampling humidity_oversampling;

    BME280_Filter filter;
    BME280_Standby standby;
    BME280_Mode mode;

} BME280_Config;

/* Driver instance: one instance per physical sensor */
typedef struct
{
    uint8_t address;

    BME280_Calibration calib;
    BME280_Config config;

    double t_fine;
    uint8_t initialized;

} BME280_Handle;

/* Default configuration */
void BME280_GetDefaultConfig(BME280_Config *config);

/* Initialize, reset, identify, read calibration and configure */
BME280_Status BME280_Init(BME280_Handle *dev, uint8_t address, const BME280_Config *config);

/* Read uncompensated data */
BME280_Status BME280_ReadRawData(BME280_Handle *dev, BME280_RawData *raw);

/* Read and return compensated physical measurements */
BME280_Status BME280_ReadMeasurements(BME280_Handle *dev, BME280_Data *data);

/* Trigger a forced measurement and wait for completion */
BME280_Status BME280_TriggerForcedMeasurement(BME280_Handle *dev);

#endif /* BME280_H */
