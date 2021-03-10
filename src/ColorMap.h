#ifndef _H_COLOR_MAP_
#define _H_COLOR_MAP_

/*
   Get ColorMap R,G,B value 
   of input data in range [-1,1]
*/

namespace ColorMap {

  inline void GetHotToCold(double x, int* r, int* g, int* b) {
    x *= 128;
    x += 128;
    *r = x > 128 > 0 ? 2 * (x - 128) : 0;
    *g = x > 127 ? 255 - 2 * (x - 128) : 2 * x;
    *b = x < 128 ? 255 - 2 * x : 0;
  }

  inline void GetJet(double x, int* r, int* g, int* b) {
    double t_r = 0, t_g = 0, t_b = 0;

    double t1, t2;
    t1 = 0.75;
    t2 = 0.25;
    // [-1,-0.75]
    if (x < -t1) {
      t_r = 0;
      t_g = 0;
      t_b = 2.5 + 2 * x;
      // [-0.75,-0.25]
    }
    else if (x < -t2) {
      t_r = 0;
      t_g = 1.5 + 2 * x;
      t_b = 1;
      // [-0.25,0.25]
    }
    else if (x < t2) {
      t_r = 0.5 + 2 * x;
      t_g = 1;
      t_b = 0.5 - 2 * x;
      // [0.25,0.75]
    }
    else if (x < t1) {
      t_r = 1;
      t_g = 1.5 - 2 * x;
      t_b = 0;
      // [0.75,1]
    }
    else {
      t_r = 2.5 - 2 * x;
      t_g = 0;
      t_b = 0;
    }
    *r = (int)(t_r * 255);
    *g = (int)(t_g * 255);
    *b = (int)(t_b * 255);
  }
}
#endif
