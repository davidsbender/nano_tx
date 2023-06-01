#include <stdint.h>

typedef struct {
	int16_t id;
    int16_t t;
    int16_t ax;
    int16_t ay;
    int16_t az;
    int16_t gx;
    int16_t gy;
    int16_t gz;
} LSM6DSV16X_DATA;

extern LSM6DSV16X_DATA lsm6dsv16xData;

int LSM6DSV16X_Init(void);
int LSM6DSV16X_ReadDiagnostic(void);
int LSM6DSV16X_ReadTAG(void);
