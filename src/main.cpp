//#include "wobjectdefs.h"

#include <stdio.h>


#include <QApplication>
#include <QCoreApplication>

#include "KAnalysis.h"

int main(int argc, char *argv[]){

	QApplication app(argc,argv);
  QCoreApplication::addLibraryPath("./lib");

  KAnalysis widget;

  widget.show();
  

 return app.exec();
}
