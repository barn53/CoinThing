#pragma once

namespace b64 {
int encode(char* output, char* input, int inputLength);
int decode(char* output, char* input, int inputLength);
int encodedLength(int plainLength);
int decodedLength(char* input, int inputLength);

inline void fromA3ToA4(unsigned char* A4, unsigned char* A3);
inline void fromA4ToA3(unsigned char* A3, unsigned char* A4);
inline unsigned char lookupTable(char c);
} // namespace b64
