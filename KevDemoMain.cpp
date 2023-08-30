#include "KevDemoMainWindow.h"
#include "KevDemoEVCharger.h"
#include "KevDemoServerConn.h"
#include <QApplication>

#define SERVER_IP "168.0.0.1"

int main(int argc, char *argv[])
{
    // a dummy scheduling server for future usage
    KevDemoServerConn conn;
    if(conn.open(QString(SERVER_IP)) != KEV_SUCCESS)
    {
        printf("Server open failed");
        return -1;
    }

    // an electric vehicle charger for demo
    KevDemoEVCharger evc;
    if(evc.connectToServer(&conn) != KEV_SUCCESS) {
        printf("Connection to a scheduling server failed");
        return -2;
    }
    evc.setChargingCostPerKw(80);         // 80 won / 1 Kw
    evc.setChargingKwPerMin(200.0/60);    // 100 Kw / 1 hour

    QApplication a(argc, argv);
    KevDemoMainWindow w(&evc);
    w.show();

    return a.exec();
}
