#include "KAnalysis.h"
#include "KSpecWidget.h"
#include "KSpectrogram.h"


#define FLOAT_CAST reinterpret_cast<float*>
#define SHORT_CAST reinterpret_cast<short*>

 KAnalysis::KAnalysis():
  btn_play("Play"),
  btn_stop("stop"),
  btn_load("Load"),
  btn_close("Close All"),
  label_scale("scale"),
  label_window("window"),
  label_sample_rate("samplerate[wav]"),
  width(0),
  height(0),
  num_spec(0),
  scale(1),
  playing(false),
  isHide(false),
  id_max(0),
  buffer_play(nullptr)
  {
	 sp = nullptr;
 t_indicate=nullptr;
 setUpdatesEnabled(true);
 setAcceptDrops(true);

 resize(600, 400);

 setStyleSheet("\
			QWidget{background:rgb(245, 186, 184);}\
      QLabel{background:white;border: 1px solid black;}\
      QComboBox{background:white;}\
      QPushButton{color:black;}\
      QLabel:disabled{color:gray;}\
      QPushButton:disabled{color:gray;}\
      \
      ");

 /* Default Values */
 frame_size = 512;
 shift_size = 128;
 sample_rate = 16000;

 /*** Processor ***/
  logspec = new LogSpec(max_channels,frame_size);
  stft = new STFT(max_channels, frame_size, shift_size);

  //setStyleSheet(_BG_COLOR_2_);

/*** Button Section ***/
  layout_button.addWidget(&btn_play);
 
  QObject::connect(
      &btn_play, 
      &QPushButton::clicked, 
      [&]() {if(!playing)Play(); }
  );

  QObject::connect(&btn_stop, &QPushButton::clicked, [&]() 
      { Stop(); }
  );
  layout_button.addWidget(&btn_stop);

  layout_button.addWidget(&btn_load);
  QObject::connect(&btn_load, &QPushButton::clicked, [&]() {
                  QString fileName;
                  QFileDialog dialog;

                  /* signal - slot for import button */
                  fileName = dialog.getOpenFileName(this,
                      tr("Open Wav File"), ".", tr("something (*.wav)"));
                     LoadFile(fileName.toStdString().c_str());
                    }
  );

  layout_button.addWidget(&btn_close);
  QObject::connect(&btn_close, &QPushButton::clicked, [&]() { CloseAll(); } );

  
  widget_button.setLayout(&layout_button);
  layout_play.addWidget(&widget_button,BorderLayout::North);

/*** Spectrogram Section ***/

  // void QLayout::setContentsMargins(int left, int top, int right, int bottom);
  layout_spec.setContentsMargins(10,5,10,0);
  
  layout_spec.setAlignment(Qt::AlignTop);
  widget_spec.setLayout(&layout_spec);
  
  area_spec.setWidget(&widget_spec);
  //area_spec.setStyleSheet(_BG_COLOR_3_);
  
  layout_play.addWidget(&area_spec,BorderLayout::Center);

  /* Configuration Widget*/ {
    layout_param.addWidget(&label_scale);

    /* Scale ComboBox */

    combo_scale.addItem("1");
    combo_scale.addItem("2");
    combo_scale.addItem("4");
    combo_scale.addItem("8");

    combo_scale.setCurrentIndex(0);
    QObject::connect(&combo_scale, &QComboBox::currentTextChanged,
      [&](const QString& text) {
        scale = text.toInt();
      }
    );

    layout_param.addWidget(&combo_scale);
    layout_param.addWidget(&label_window);

    /* window ComboBox */
    combo_window.addItem("128/512");
    combo_window.addItem("256/1024");
    combo_window.addItem("512/2048");

    // https://doc.qt.io/qt-5/qcombobox.html#currentIndexChanged
    QObject::connect(&combo_window, QOverload<int>::of(&QComboBox::currentIndexChanged),
      [&](int index) {
        switch (index) {
        case 0:
          shift_size = 128;
          frame_size = 512;
          break;
        case 1:
          shift_size = 256;
          frame_size = 1024;
          break;
        case 2:
          shift_size = 512;
          frame_size = 2048;
          break;
        }
        delete stft, logspec;
        stft = new STFT(max_channels, frame_size, shift_size);
        logspec = new LogSpec(max_channels, frame_size);
      }
    );
    layout_param.addWidget(&combo_window);

    /* sample_rate combobox */
    layout_param.addWidget(&label_sample_rate);
    combo_sample_rate.addItem("16000");
    combo_sample_rate.addItem("22500");
    combo_sample_rate.addItem("48000");
    combo_sample_rate.addItem("96000");

    QObject::connect(&combo_sample_rate, QOverload<int>::of(&QComboBox::currentIndexChanged),
      [&](int index) {
        switch (index) {
        case 0:sample_rate = 16000;
          break;
        case 1:sample_rate = 22500;
          break;
        case 2:sample_rate = 48000;
          break;
        case 3:sample_rate = 96000;
          break;
        }
      }
    );

    layout_param.addWidget(&combo_sample_rate);
    widget_param.setLayout(&layout_param);
    layout_config.addWidget(&widget_param);
  }


  /* Output Device Selection Widget */ {
    widget_output.setLayout(&layout_output);
    layout_output.addWidget(&btn_AudioProbe);
    layout_output.addWidget(&label_device);
    layout_output.addWidget(&combobox_device);
    //layout_output.addWidget(&label_channels);
    //layout_output.addWidget(&combobox_channels);
    layout_output.addWidget(&label_samplerate);
    layout_output.addWidget(&combobox_samplerate);
    layout_config.addWidget(&widget_output);

    btn_AudioProbe.setText("AuidoProbe[Output]");
    label_device.setText("device");
    label_device.setFixedWidth(60);
    combobox_device.setFixedWidth(300);
    //label_channels.setText("channels");
    //label_channels.setFixedWidth(60);
    //combobox_channels.setFixedWidth(60);
    label_samplerate.setText("samplerate[output]");
    label_samplerate.setFixedWidth(130);

    output_device = 0;
    //output_channels = 1;
    output_samplerate= 48000;


    QObject::connect(&btn_AudioProbe, &QPushButton::pressed, this, &KAnalysis::SlotAudioProbe);
    QObject::connect(&combobox_device, &QComboBox::currentIndexChanged, this,&KAnalysis::SlotChangeDevice);
    QObject::connect(&combobox_samplerate, &QComboBox::currentIndexChanged, this,&KAnalysis::SlotChangeSamplerate);

    SlotAudioProbe();
  }
  layout_config.addStretch();
  
  widget_play.setLayout(&layout_play);
  widget_config.setLayout(&layout_config);
  this->addTab(&widget_play, "Play");
  this->addTab(&widget_config, "Config");
}

KAnalysis::~KAnalysis(){
  if(buffer_play) {
    switch(fmt_type){
      case 3:
        delete[] FLOAT_CAST(buffer_play);
        break;
      default :
        delete[] SHORT_CAST(buffer_play);
        break;
    }
  }
  if(t_indicate && t_indicate->joinable())
    t_indicate->join();
  if (t_indicate) {
    delete t_indicate;
  }
  delete stft,logspec,sp;
}

/* dragEnter?êÏÑú   */
 void KAnalysis::dragEnterEvent(QDragEnterEvent *event){
  /* Recognize  dropEvent, only After  acceptProposedAction. */
  event->acceptProposedAction();

}

 void KAnalysis::dropEvent(QDropEvent * event){
  std::string temp_str;

  /* Dose Dropped file have Url? */
  if (event->mimeData()->hasUrls()){
    QList<QUrl> url = event->mimeData()->urls();

    //for (int i = 0; i < url.size(); ++i) {
    // Get Urls as QList but gonna use only one url.
    for (int i = 0; i < 1; ++i) {
      // std::cout<<( temp.toString() ).toStdString()<<"\n";
      /* ex) file:///home/kbh/git/IIP_demo/build/input.wav */
      /* file://<path> format  */

      QUrl temp_qurl =  url.at(i);
      temp_str = (temp_qurl.toString() ).toStdString();
	 // std::cout << "temp_str "<<temp_str << "\n";
#ifdef _WIN32
	  /*
		 in Windows , getOpenFileName has
		 '/' before path. 
		 ex)
		 WIN32 : file:///C:/folder/file.wav
		 else  : file://C:/folder/file.wav
	  */
	  
	  temp_str = temp_str.substr(8);
#else
	  temp_str = temp_str.substr(7);
#endif
	  LoadFile(temp_str.c_str());
    }
  }
}

 void KAnalysis::LoadFile(const char* file_name){
  if(!IsWavFile((char*)file_name))
    printf("ERROR::Not a wav file.\n");

  WAV wav_tmp;
  wav_tmp.SetSizes(frame_size, shift_size);
  std::cout << "KAnalysis::LoadFile(" << file_name << ")" << std::endl;

  wav_tmp.OpenFile(file_name);
  fmt_type = wav_tmp.GetFmtType();

  int channels = wav_tmp.GetChannels();
  int sample_rate = wav_tmp.GetSampleRate();
  int length = wav_tmp.GetSize();

  double* max_abs = new double[channels];
  memset(max_abs, 0, sizeof(double) * channels);
  double* max_val = new double[channels];
  memset(max_val, 0, sizeof(double) * channels);
  double* min_val = new double[channels];
  memset(min_val, 0, sizeof(double) * channels);

  /* Adjusting in case sample size doesn't be devided by sihft_size */
  if (length % (channels * 2 * shift_size) != 0) {
    length /= channels * 2 * shift_size;
    length += 1;
  }
  else
    length /= channels * 2 * shift_size;

#ifndef NDEBUG
  printf("NOTE::the number of samples : %d * shift_size\n", length);
#endif

  /* Initialize Spectrogram Widgets */
  for (int i = 0; i < channels; i++) {
    KSpecWidget* temp_spec
      = new KSpecWidget(this,
        // fmt is fixed for short(1)
        i + 1, length, frame_size + 2, shift_size, sample_rate, scale, 1, id_max++);
    vector_spec.push_back(temp_spec);
    num_spec++;
  }

  /* Initialize buffers */
  double** buf_raw;
  buf_raw = new double* [channels];
  for (int i = 0; i < channels; i++) {
    buf_raw[i] = new double[frame_size];
    memset(buf_raw[i], 0, (frame_size) * sizeof(double));
  }
  double*** buf_data;
  buf_data = new double** [length];
  for (int i = 0; i < length; i++) {
    buf_data[i] = new double* [channels];
    for (int j = 0; j < channels; j++) {
      buf_data[i][j] = new double[frame_size + 2];
      memset(buf_data[i][j], 0, (frame_size + 2) * sizeof(double));
    }
  }

  /* save buffer */
  for (int j = 0; j < length; j++) {
    wav_tmp.Convert2Array(buf_raw);
    /* give wav buffer for each specwidget */
  #pragma omp parallel for
    for (int i = 0; i < channels; i++) {
      short* temp_buf =
        SHORT_CAST(vector_spec.at(num_spec - channels + i)->Get_buffer_wav());
      for (int k = 0; k < shift_size; k++)
        temp_buf[j * shift_size + k] =
        static_cast<short>(buf_raw[i][k]);
    }
    // Get min,max value for normalization
    for (int i = 0; i < channels; i++)
      for (int k = 0; k < shift_size; k++) {
        max_abs[i] = std::max(max_abs[i], std::abs(buf_raw[i][k]));
      }
  }

  // reset
  wav_tmp.Rewind();
  for (int i = 0; i < channels; i++)
    memset(buf_raw[i], 0, (frame_size) * sizeof(double));

    int caxis_max = 20;
    int caxis_min = -100;
  /* Create Spectrogram */
  for (int j = 0; j < length; j++){
    wav_tmp.Convert2Array(buf_raw);

    /* Noramlization */

#pragma omp parallel for
    for (int k = 0; k < shift_size; k++) {
      for (int i = 0; i < channels; i++) {
        buf_raw[i][k] = buf_raw[i][k] * (32767.0 / max_abs[i]);
      }
    }

  stft->stft(buf_raw, buf_data[j], channels);
    logspec->Process(buf_data[j], channels);
 

   // caxis
    for (int i = 0; i < channels; i++) {
      for (int k = 0; k < frame_size / 2 + 1; k++) {
        if (buf_data[j][i][k] < caxis_min) buf_data[j][i][k] = caxis_min;
        if (buf_data[j][i][k] > caxis_max) buf_data[j][i][k] = caxis_max;
      }
    }
  }

  for (int i = 0; i < channels; i++)
    //vector_spec.at(num_spec - channels + i)->SetRange(min_val[i], max_val[i]);
    vector_spec.at(num_spec - channels + i)->SetRange(caxis_min, caxis_max);

  /* Set Spectrograms pixels */
  for (int i = 0; i < channels; i++)
    for (int j = 0; j < length; j++)
      vector_spec.at(num_spec - channels + i)->Update(buf_data[j][i]);

  /* Display Spectrograms */
  for (int i = 0; i < channels; i++) {
    KSpecWidget* temp_spec = vector_spec.at(num_spec - channels + i);
    temp_spec->Confirm();
    layout_spec.addWidget((QWidget*)temp_spec);

    /* Set width as max of specs */
    if (width < temp_spec->GetSpecWidth())
      width = temp_spec->GetSpecWidth();
    height += temp_spec->GetSpecHeight() + 10;
    widget_spec.resize(width, height);
  }
  wav_tmp.Finish();
  for (int i = 0; i < channels; i++) {
    delete[] buf_raw[i];
    delete[] buf_data[i];
  }
  delete[] buf_data;
  delete[] buf_raw;
  delete[] max_val;
  delete[] min_val;

  update();
 }

 bool KAnalysis::IsWavFile(char* file_name) {
   char* temp;
   char str_buf[128];
   strcpy(str_buf, file_name);
   bool ret;
   temp = strtok(str_buf, ".");
   temp = strtok(nullptr, ".");
   if (temp == nullptr) {
     printf("ERROR::File without file extension.\n");
     return false;
   }
   ret = !strcmp(temp, "wav");
   return ret;
 }


 void KAnalysis::CloseSpec(KSpecWidget* target) {
   /* Find */
   auto it =
     std::find(vector_spec.begin(), vector_spec.end(), target);
   auto dist = distance(vector_spec.begin(), it);
   /* delete & adjust */
   vector_spec.erase(vector_spec.begin() + dist);
   layout_spec.removeWidget((QWidget*)target);
   height -= target->GetSpecHeight() + 10;
   delete target;
   widget_spec.resize(width, height);
   num_spec--;
 }

 /* ?¨ÏÉù ?úÏãú */
 void KAnalysis::Indicate() {
   int delay = (1000000 / (double)sample_rate) * shift_size;

   for (int i = 0; i < vector_spec.size(); i++) {
     vector_spec.at(i)->SetDrawable(false);
     QWidget::connect(this, &KAnalysis::IndicateSignal,
       vector_spec.at(i), &KSpecWidget::Indicator);
     //    vector_spec.at(i)->Refresh(); 
   }
   for (int j = from; j < to; j++) {

     auto m_start = std::chrono::high_resolution_clock::now();

     emit(IndicateSignal(j));
     /*
       for(int i=0;i<vector_selected.size();i++) {
         vector_selected.at(i)->Indicator(j);
       }
       */


     auto m_elapsed
       = std::chrono::duration_cast<std::chrono::microseconds>(
         std::chrono::high_resolution_clock::now() - m_start);
     auto microseconds
       = std::chrono::duration_cast<std::chrono::microseconds>(m_elapsed).count();
     std::chrono::microseconds timespan_micro(delay - microseconds);
     /* to synchronize play. wait for (expected time - elapsed time).
        since computation, will be microsecond error.
     */
     std::this_thread::sleep_for(timespan_micro);

     if (!playing)break;
   }
   for (auto i = 0; i < vector_spec.size(); i++) {
     vector_spec.at(i)->SetDrawable(true);
     QWidget::disconnect(this, &KAnalysis::IndicateSignal, 0, 0);
   }
   playing = false;
   if (sp->IsRunning())sp->Stop();
   delete sp;
   emit(SignalStopPlay());
   printf("Indicator Destructed\n");
 }


 void KAnalysis::SetArea(QPoint global, QPoint origin) {
   for (int i = 0; i < vector_spec.size(); i++)
     vector_spec.at(i)->SetArea(global, origin);
 }

 void KAnalysis::RefreshSpecs() {
   for (int i = 0; i < vector_spec.size(); i++)
     vector_spec.at(i)->Refresh();
 }

 void KAnalysis::SetRange(int _from, int _to) {
   /* emptying vector */
   vector_selected.clear();
   from = _from;
   to = _to;
#ifndef NDEBUG
   /*
    printf("KAnalysis::SetRange [%-4d  %-4d]\n",from,to);
     for(int i=0;i<vector_spec.size();i++){
       if(vector_spec.at(i)->IsInArea())
         printf("ID : %d is in area.\n",vector_spec.at(i)->Get_id());
     }
     */
#endif
 }

 inline void KAnalysis::Play() {
   // playing = true;
   channel_play = 0;
   size_play = to - from;
   for (int i = 0; i < vector_spec.size(); i++) {
     if (vector_spec.at(i)->IsInArea()) {
       vector_selected.push_back(vector_spec.at(i));
       channel_play++;
     }
   }
   /*
   printf("2. size of vector : %d\n",vector_selected.size());
   printf("size_buf %d * %d * %d = %d\n",
       channel_play,
       size_play,
       shift_size,
       channel_play * size_play * shift_size
       );
       */

       /* Nothnig is sellected*/
   if (channel_play == 0) {
     // empty
     if (vector_spec.size() == 0)return;
#ifndef NDEBUG
     printf("KAnalysis::Play::play first spectrogram\n");
#endif

     // play first one.
     channel_play = 1;
     size_play = vector_spec.at(0)->GetSpecWidth();
     from = 0;
     to = size_play;

     fmt_type = vector_spec.at(0)->Get_fmt_type();

     switch (fmt_type) {
     case 3:
       if (buffer_play) delete[] FLOAT_CAST(buffer_play);
       buffer_play = new float[1 * size_play * shift_size];
       for (int i = 0; i < size_play * shift_size; i++) {
         if (i > (vector_spec.at(0)->Get_length()) * shift_size)
           FLOAT_CAST(buffer_play)[i] = 0;
         else
           FLOAT_CAST(buffer_play)[i]
           = FLOAT_CAST(vector_spec.at(0)->Get_buffer_wav())[i];
       }
       break;

     default:
       if (buffer_play) delete[] SHORT_CAST(buffer_play);
       buffer_play = new short[1 * size_play * shift_size];
       for (int i = 0; i < size_play * shift_size; i++) {
         if (i > (vector_spec.at(0)->Get_length()) * shift_size)
           SHORT_CAST(buffer_play)[i] = 0;
         else
           SHORT_CAST(buffer_play)[i]
           = SHORT_CAST(vector_spec.at(0)->Get_buffer_wav())[i];
       }
       break;

     }

     /* Normal Behavior */
   }
   else {
     /* Pick first specWidget as representing type */
     fmt_type = vector_selected.at(0)->Get_fmt_type();

     //  printf("play fmt : %d\n",fmt_type);
       /* Create buffer for play */
     switch (fmt_type) {
     case 3:
#ifndef NDEBUG
       printf("INFO:: build float buffer\n");
#endif
       if (buffer_play) delete[] FLOAT_CAST(buffer_play);
       buffer_play = new float[channel_play * size_play * shift_size];
       for (int i = 0; i < size_play * shift_size; i++) {
         for (int j = 0; j < vector_selected.size(); j++) {
           if (i > (vector_selected.at(j)->Get_length()) * shift_size)
             FLOAT_CAST(buffer_play)[i * channel_play + j] = 0;
           else
             FLOAT_CAST(buffer_play)[i * channel_play + j]
             = FLOAT_CAST(vector_selected.at(j)->Get_buffer_wav())[i + from * shift_size];
         }
       }
       break;
     default:
#ifndef NDEBUG
       printf("INFO:: build short buffer\n");
#endif
       if (buffer_play) delete[] SHORT_CAST(buffer_play);
       buffer_play = new short[channel_play * size_play * shift_size];
       for (int i = 0; i < size_play * shift_size; i++) {
         for (int j = 0; j < vector_selected.size(); j++) {
           if (i > (vector_selected.at(j)->Get_length()) * shift_size)
             SHORT_CAST(buffer_play)[i * channel_play + j] = 0;
           else
             SHORT_CAST(buffer_play)[i * channel_play + j]
             = SHORT_CAST(vector_selected.at(j)->Get_buffer_wav())[i + from * shift_size];
         }
       }
       break;
     }

   }


   /*** Scaling ***/
   /* Get Peak */
   short peak_short_buf = 100;
   float peak_float_buf = 0.5;

   const int peak_start_idx = 8000;

   int ii;
   /* Search max peak value */
#pragma omp parallel for private(ii)
   for (ii = peak_start_idx; ii < channel_play * size_play * shift_size; ii++) {
     switch (fmt_type) {
     case 3:
       if (peak_float_buf < fabs(FLOAT_CAST(buffer_play)[ii]))
         peak_float_buf = fabs(FLOAT_CAST(buffer_play)[ii]);
       break;
     default:
       if (peak_short_buf < abs(SHORT_CAST(buffer_play)[ii]))
         peak_short_buf = abs(SHORT_CAST(buffer_play)[ii]);
       break;
     }
   }


   switch (fmt_type) {
   case 3:
     printf("peak %f\n", peak_float_buf);
     break;
   default:
     printf("peak %d %d\n", peak_short_buf, 32768 / peak_short_buf);
     break;
   }
   /* norm & scale */
#pragma omp parallel for private(ii)
   for (ii = 0; ii < channel_play * size_play * shift_size; ii++) {
     switch (fmt_type) {
     case 3:
       FLOAT_CAST(buffer_play)[ii] / peak_float_buf;
       break;

     default:
       SHORT_CAST(buffer_play)[ii] *= (32768 / peak_short_buf);
       break;
       break;
     }
   }



   /*
    * audio play
    * */

   if (!buffer_play) {
     std::cout << "buffer_play is NULL!!" << std::endl;
     return;
   }

   if (channel_play != 0) {
     sp = new RtOutput(output_device, channel_play, sample_rate, output_samplerate, shift_size, frame_size);
     switch (fmt_type) {
     case 3:
       sp->FullBufLoad(FLOAT_CAST(buffer_play), (long)(channel_play * size_play * shift_size));
       break;
     default:
       sp->FullBufLoad(SHORT_CAST(buffer_play), (long)(channel_play * size_play * shift_size));
       break;
     }
     sp->Start();
     emit(SignalStartPlay());
   }

   /* Indicator == Destructor for sp */

   playing = true;
   if (t_indicate)delete t_indicate;
   t_indicate = new std::thread(&KAnalysis::Indicate, this);
   t_indicate->detach();
   //delete[] buffer_play;

 }

 inline void KAnalysis::Stop() {
   if (playing) {
     playing = false;
   }
   RefreshSpecs();
 }

 void KAnalysis::CloseAll() {
   while (vector_spec.size() != 0) {
     KSpecWidget* temp = vector_spec.at(0);
     CloseSpec(temp);
   }
 }

 /* Perform AudioProbe and set 
    widget_output as probed audio device
 */
 void KAnalysis::SlotAudioProbe() {

   /* Clear */
   //printf("combobox count : %d, combobox index : %d\n", combobox_device.count(),combobox_device.currentIndex());

   combobox_device.clear();
     //combobox_channels.clear();
   combobox_samplerate.clear();
   while (!vector_device.empty()){
    device* tmp = vector_device.back();
    tmp->samplerate.clear();
    vector_device.pop_back();
    delete tmp;
 }

   // From default RtAudio AudioProbe 
   RtAudio audio;
   unsigned int devices = audio.getDeviceCount();
   RtAudio::DeviceInfo info;
   for (unsigned int i = 0; i < devices; i++) {
     info = audio.getDeviceInfo(i);
     // only for output devices
     if (info.probed == true &&  info.outputChannels > 0) {
       device *temp;
       temp = new device;
       vector_device.push_back(temp);
       temp->number = i;
       temp->name = info.name;
       //temp->channels = info.outputChannels;
       for (auto sr : info.sampleRates)
         temp->samplerate.push_back(sr);
     }
   }

   for (auto dv : vector_device) {
     combobox_device.addItem(QString::fromStdString(dv->name));
   }

   /* Default Device*/
   SlotChangeDevice(0);
}


 void KAnalysis::SlotChangeDevice(int index) {
  // printf("Slot Change Device : %d\n", index);
   if (index < 0)
     return;
   device* temp = vector_device.at(index);
   output_device = temp->number;
   //combobox_channels.clear();
   combobox_samplerate.clear();

   //for (int i = 1; i <= temp->channels; i++)
   //  combobox_channels.addItem(QString::number(i));
   int idx = 0;
   bool default48k=false;
   for (auto i : temp->samplerate) {
     combobox_samplerate.addItem(QString::number(i));

     /* Default Samplerate as 48000 */
     if (i == 48000) {
       output_samplerate = i;
       combobox_samplerate.setCurrentIndex(idx);
       default48k = true;
     }
     idx++;
   }

   if (!default48k) {
     combobox_samplerate.setCurrentIndex(0);
     output_samplerate = combobox_samplerate.itemText(0).toInt();
   }

   emit(SignalSetSoundplayInfo(output_device, output_samplerate));
}

 void KAnalysis::SlotChangeSamplerate(int index) {
   QString tmp = combobox_samplerate.itemText(index);
   output_samplerate = tmp.toInt();
   emit(SignalSetSoundplayInfo(output_device, output_samplerate));
 }

 void KAnalysis::ToggleHide() {
   if (!isHide) {
     printf("KAnalysis::ToggleHide::hide\n");
     isHide = true;
   }
   else {
     printf("KAnalysis::ToggleHide::show\n");
     isHide = false;
   }
 }

