int strToInt(char* str, int* valid, char** end)
{
    int value = 0;
    int sign = 0;
    int i = 0;
    while (str[i] != '\0') {
        if ((sign == 0) && (str[i] == '+')) {
            sign = +1;
        } else if ((sign == 0) && (str[i] == '-')) {
            sign = -1;
        } else if ((str[i] >= '0') && (str[i] <= '9')) {
            value = value * 10 + (int)(str[i] - '0');
            if (value & 0x80000000) {
                *valid = 0;
            }
        } else {
            break;
        }
        i++;
    }
    *end = &str[i];
    if (sign == -1) {
        value = -value;
    }
    return value;
}
