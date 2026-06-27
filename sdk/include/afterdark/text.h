// Built-in 3x5 bitmap font + text drawing.
//
// Text is drawn purely through Canvas::fill_rect, so it works on every backend
// (SDL2, native hosts, headless/CI) with zero per-backend code and no asset
// files. Good enough for HUDs, labels, and text-based modules (Clocks,
// Messages). For large display type a real font atlas can come later.
#pragma once

#include <cstdint>
#include <string>

#include "afterdark/afterdark.h"

namespace ad {

// Each glyph is 5 rows; the low 3 bits of each byte are the columns (bit2=left).
inline const uint8_t* glyph3x5(char c) {
  // clang-format off
  static const uint8_t SP[5] = {0,0,0,0,0};
  static const uint8_t D0[5] = {0b111,0b101,0b101,0b101,0b111};
  static const uint8_t D1[5] = {0b010,0b110,0b010,0b010,0b111};
  static const uint8_t D2[5] = {0b111,0b001,0b111,0b100,0b111};
  static const uint8_t D3[5] = {0b111,0b001,0b111,0b001,0b111};
  static const uint8_t D4[5] = {0b101,0b101,0b111,0b001,0b001};
  static const uint8_t D5[5] = {0b111,0b100,0b111,0b001,0b111};
  static const uint8_t D6[5] = {0b111,0b100,0b111,0b101,0b111};
  static const uint8_t D7[5] = {0b111,0b001,0b010,0b010,0b010};
  static const uint8_t D8[5] = {0b111,0b101,0b111,0b101,0b111};
  static const uint8_t D9[5] = {0b111,0b101,0b111,0b001,0b111};
  static const uint8_t A[5]  = {0b111,0b101,0b111,0b101,0b101};
  static const uint8_t B[5]  = {0b110,0b101,0b110,0b101,0b110};
  static const uint8_t C[5]  = {0b111,0b100,0b100,0b100,0b111};
  static const uint8_t D[5]  = {0b110,0b101,0b101,0b101,0b110};
  static const uint8_t E[5]  = {0b111,0b100,0b110,0b100,0b111};
  static const uint8_t F[5]  = {0b111,0b100,0b110,0b100,0b100};
  static const uint8_t G[5]  = {0b111,0b100,0b101,0b101,0b111};
  static const uint8_t H[5]  = {0b101,0b101,0b111,0b101,0b101};
  static const uint8_t I[5]  = {0b111,0b010,0b010,0b010,0b111};
  static const uint8_t J[5]  = {0b001,0b001,0b001,0b101,0b111};
  static const uint8_t K[5]  = {0b101,0b101,0b110,0b101,0b101};
  static const uint8_t L[5]  = {0b100,0b100,0b100,0b100,0b111};
  static const uint8_t M[5]  = {0b101,0b111,0b111,0b101,0b101};
  static const uint8_t N[5]  = {0b101,0b111,0b111,0b111,0b101};
  static const uint8_t O[5]  = {0b111,0b101,0b101,0b101,0b111};
  static const uint8_t P[5]  = {0b111,0b101,0b111,0b100,0b100};
  static const uint8_t Q[5]  = {0b111,0b101,0b101,0b111,0b011};
  static const uint8_t R[5]  = {0b111,0b101,0b110,0b101,0b101};
  static const uint8_t S[5]  = {0b111,0b100,0b111,0b001,0b111};
  static const uint8_t T[5]  = {0b111,0b010,0b010,0b010,0b010};
  static const uint8_t U[5]  = {0b101,0b101,0b101,0b101,0b111};
  static const uint8_t V[5]  = {0b101,0b101,0b101,0b101,0b010};
  static const uint8_t W[5]  = {0b101,0b101,0b111,0b111,0b101};
  static const uint8_t X[5]  = {0b101,0b101,0b010,0b101,0b101};
  static const uint8_t Y[5]  = {0b101,0b101,0b010,0b010,0b010};
  static const uint8_t Z[5]  = {0b111,0b001,0b010,0b100,0b111};
  static const uint8_t COL[5]= {0b000,0b010,0b000,0b010,0b000};
  static const uint8_t DSH[5]= {0b000,0b000,0b111,0b000,0b000};
  static const uint8_t DOT[5]= {0b000,0b000,0b000,0b000,0b010};
  static const uint8_t SLH[5]= {0b001,0b001,0b010,0b100,0b100};
  static const uint8_t BNG[5]= {0b010,0b010,0b010,0b000,0b010};
  // clang-format on
  if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
  switch (c) {
    case ' ': return SP;
    case '0': return D0; case '1': return D1; case '2': return D2;
    case '3': return D3; case '4': return D4; case '5': return D5;
    case '6': return D6; case '7': return D7; case '8': return D8;
    case '9': return D9;
    case 'A': return A; case 'B': return B; case 'C': return C;
    case 'D': return D; case 'E': return E; case 'F': return F;
    case 'G': return G; case 'H': return H; case 'I': return I;
    case 'J': return J; case 'K': return K; case 'L': return L;
    case 'M': return M; case 'N': return N; case 'O': return O;
    case 'P': return P; case 'Q': return Q; case 'R': return R;
    case 'S': return S; case 'T': return T; case 'U': return U;
    case 'V': return V; case 'W': return W; case 'X': return X;
    case 'Y': return Y; case 'Z': return Z;
    case ':': return COL; case '-': return DSH; case '.': return DOT;
    case '/': return SLH; case '!': return BNG;
    default:  return nullptr;  // unknown glyphs render as a gap
  }
}

// Pixel width of a string at the given scale (3px glyph + 1px gap per char).
inline int text_width(const std::string& s, int scale) {
  return static_cast<int>(s.size()) * 4 * scale;
}

// Draw text with the top-left of the first glyph at (x, y).
inline void draw_text(Canvas& cv, int x, int y, const std::string& s, int scale,
                      Color col) {
  int cx = x;
  for (char ch : s) {
    const uint8_t* g = glyph3x5(ch);
    if (g) {
      for (int row = 0; row < 5; ++row)
        for (int colb = 0; colb < 3; ++colb)
          if (g[row] & (1 << (2 - colb)))
            cv.fill_rect(cx + colb * scale, y + row * scale, scale, scale, col);
    }
    cx += 4 * scale;
  }
}

}  // namespace ad
