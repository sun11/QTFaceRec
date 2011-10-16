#include "thread.h"
#include "ui_mainwindow.h"

Thread::Thread(QObject *parent)
    :QThread(parent)
{
}

void Thread::run()
{
    while(1){
    emit sigtest();
//    printf("Hello from Thread!\n");
    QThread::msleep(500);
    }
}
