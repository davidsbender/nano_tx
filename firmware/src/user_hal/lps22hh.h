#include <stdint.h>

typedef struct {
	int id;
    uint32_t p;
    int16_t t;
} LPS22HH_DATA;

extern LPS22HH_DATA lps22hhData;

void LPS22HH_Init(void);
int LPS22HH_ReadDiagnostic(void);
int LPS22HH_ReadPressure(void);
