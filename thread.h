#ifndef THREAD_H
#define THREAD_H

#include <QThread>
//#include "mainwindow.h"

class Thread : public QThread
{
    Q_OBJECT

public:
   Thread(QObject *parent=0);

protected:
   void run();

signals:
   void sigtest();
};


#endif // THREAD_H
