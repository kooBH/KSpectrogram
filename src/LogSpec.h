#ifndef _H_LOGSPEC_
#define _H_LOGSPEC_

#include <cmath>

class LogSpec {
  private:
    int channels,frame_size;

  public:
    inline LogSpec(int channels,int frame_size);
    inline ~LogSpec();
    inline void Process(double**);
    inline void Process(double**,int);
    inline void Process(double*);
};

inline LogSpec::LogSpec(int _ch,int _fr){
    channels = _ch;
    frame_size = _fr;

}
inline LogSpec::~LogSpec(){}

/* matlap formula
 * imagesc(10*log10(abs(squeeze(X(sensorIndex,:,:))).^2));
 * */

inline void log_spec(double* buf,int size){
   int re;
   int im;
   double tmp=0;
   for (int i = 0; i < size; i += 2) {
     re = i;
     im = i + 1;
     tmp = std::pow(buf[re], 2) + std::pow(buf[im], 2);
     //printf("buf[%d] (%d %d) = %lf : %lf %lf\n", i / 2, re,im, tmp,buf[re],buf[im]);
     buf[i / 2] = 10 * std::log10(tmp);
     //printf("INFO::log_spec::buf[%d] : %lf\n",i/2,buf[i/2]);
   }
}

inline void LogSpec::Process(double**data){
#pragma omp parallel for
    for(int i=0;i<channels;i++)
        log_spec(data[i],frame_size + 2);
}
inline void LogSpec::Process(double**data,int ch){
#pragma omp parallel for
    for(int i=0;i<ch;i++)
        log_spec(data[i],frame_size + 2);
}
inline void LogSpec::Process(double*data){
    log_spec(data,frame_size + 2);
}

#endif
