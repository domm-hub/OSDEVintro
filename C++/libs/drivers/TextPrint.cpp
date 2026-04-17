#include "IO.h"
#include "TypeDefs.h"

char FloatToStringOutput[128];
char HexToStringOutput[128];
char IntegerToStringOutput[128];

/**
 * A simple floating point to string converter that avoids software float libraries
 * by using basic integer math for the fractional part.
 * Note: This version has limited precision and range compared to a full IEEE implementation,
 * but it avoids the __divdf3 and other linker errors.
 */
// const char* FloatToString(double value, int precision) {
//     if (precision > 10) precision = 10;
    
//     int i = 0;
//     bool isNegative = false;

//     if (value < 0) {
//         isNegative = true;
//         value = -value;
//     }

//     // 1. Extract integer part
//     unsigned long long integerPart = (unsigned long long)value;
    
//     // 2. Extract fractional part
//     double diff = value - (double)integerPart;
    
//     // Build string
//     if (isNegative) FloatToStringOutput[i++] = '-';

//     // Convert Integer Part
//     int intStart = i;
//     if (integerPart == 0) {
//         FloatToStringOutput[i++] = '0';
//     } else {
//         while (integerPart > 0) {
//             FloatToStringOutput[i++] = (integerPart % 10) + '0';
//             integerPart /= 10;
//         }
//         // Reverse integer portion
//         int start = intStart, end = i - 1;
//         while (start < end) {
//             char t = FloatToStringOutput[start];
//             FloatToStringOutput[start] = FloatToStringOutput[end];
//             FloatToStringOutput[end] = t;
//             start++; end--;
//         }
//     }

//     if (precision > 0) {
//         FloatToStringOutput[i++] = '.';
        
//         // Convert fractional part by multiplying by 10 repeatedly
//         for (int p = 0; p < precision; p++) {
//             diff *= 10.0;
//             int digit = (int)diff;
//             FloatToStringOutput[i++] = digit + '0';
//             diff -= (double)digit;
//         }
//     }

//     FloatToStringOutput[i] = '\0';
//     return FloatToStringOutput;
// }
