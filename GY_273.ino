#include <GY273.h>

template<class T> inline Print &operator <<(Print &obj, T arg)
{
    obj.print(arg);
    return obj;
}

#define cout Serial
#define endl "\n\r"

int x, y, z;

void setup()
{
        Serial.begin(9600);
        while (GY273Setup() != GY273_SUCCESS)
        {
                cout << "GY-273 setup process has failed. Retrying in 1 second..." << endl;
                delay(1000);
        }
}

void loop()
{
        GY273Read(GY273_X, &x);
        GY273Read(GY273_Y, &y);
        GY273Read(GY273_Z, &z);
        cout << "x=" << x << " y=" << y << " z=" << z << endl;
        delay(60);
}
