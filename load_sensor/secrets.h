#include <pgmspace.h>
 
#define SECRET
#define THINGNAME "LoadSensor"
 
const char WIFI_SSID[] = "AndroidAP";
const char WIFI_PASSWORD[] = "lkvi5630";
const char AWS_IOT_ENDPOINT[] = "a2ezfc6atnz7fy-ats.iot.us-east-2.amazonaws.com";
 
// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";
 
// Device Certificate
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUH+QZeTBxBlipq0z+C5L/Qw+SGSkwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTIzMTEyOTEyMTMx
N1oXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALcpo+iqLQLYZ1M+OylZ
8rBHlGV41YQAK6Yp2hwaZPhMDty9m3NxJZ+pitaMfMpg3UJ6qrD11elMqb1ybTx+
tv7XikUlSfbrkM+uxUf6wEwQjlzuQUkR03A0VN2bljMotRrHUtA7LRCP0kY2YxLA
28vgfvEvdJ0Ab57WgcHyvceZZtTVLthcnFwpqxmsDOkhRaktZ+E/k0S8EvoPYmoX
K33dC38dzJVPomQl6hRKbBTZTfU8oEGCpqshOvj033wJKTXud4B1IYMsg0VB+zbi
ggLzVMFrxzX7GgsXiA4mCTecXLzrp2lN+Vmi5cxN8d0a6bM/kyosSLyERzhFNsEu
oMsCAwEAAaNgMF4wHwYDVR0jBBgwFoAUyTqlqtkH8hMC2sCmo5wuile+iO8wHQYD
VR0OBBYEFKyI8krw77ztJa+yH3qc1s4oUh4UMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQBbcD9ug+ftTcn06yMQ7WuCgyMI
z/p+BBmjvhNKuMik8TSu4IO41TYPy7M3heNSxDrpQRFQ+KcaiaTadPNJ7yFP+9Fs
Txt0f/jvC6+Pavb3B81X7PV27Pe/8HR4x3Q38IOjiddefJ5ia8SVRapUmR+JZGdm
IwazyCe+vSIzy/fS9u78t2ddf/I8SaEZpSyg2I6Gqnkb4vwmfzujHs1waSxPbDes
/M2+EONQ8rZsFLOTsD3fem0RrgBQI7o2SRiYrAo1hb0xZ1c/YPe4GMUy6qWKbtTV
zPo1IamrnoU3Wo9RwGdyldaZzzMlUcZAC5bMtB0SV9H824DEd3oa57kMs+7u
-----END CERTIFICATE-----
)KEY";
 
// Device Private Key
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEpAIBAAKCAQEAtymj6KotAthnUz47KVnysEeUZXjVhAArpinaHBpk+EwO3L2b
c3Eln6mK1ox8ymDdQnqqsPXV6UypvXJtPH62/teKRSVJ9uuQz67FR/rATBCOXO5B
SRHTcDRU3ZuWMyi1GsdS0DstEI/SRjZjEsDby+B+8S90nQBvntaBwfK9x5lm1NUu
2FycXCmrGawM6SFFqS1n4T+TRLwS+g9iahcrfd0Lfx3MlU+iZCXqFEpsFNlN9Tyg
QYKmqyE6+PTffAkpNe53gHUhgyyDRUH7NuKCAvNUwWvHNfsaCxeIDiYJN5xcvOun
aU35WaLlzE3x3Rrpsz+TKixIvIRHOEU2wS6gywIDAQABAoIBAAjWT3w4SberbUDM
7dnO5zCuI3DJ5bNatsBroIRemaSJNuZbl5Z4TRJpAbPcT2EMT2CXvoiPM+Tvt/jV
/oM+seGV0KwJYooxT2lVy1rvtDt3SQcez7OQ0pzFSZrvmM2bFzHVkB9P25rC9ahB
k4IiH/i0Oiu02HGCxd/qBZSv+4wXu6+/A8FNYCeLdN+Qiyd3hzVVSpO0lDr+IA7v
TFL6mDck3RDDy0ud7KmpIkC/Ky/4VY32F573UfYPluPOEwtBBFnCQwasuhrWsXzA
UQa+69/IogRuUToRhNNuV19q9HortBZlx5WGRwgKdLTb8Jgx4ILRY6IUB8eC3Uxh
69G/ujECgYEA68zGO3uu2apNuUQjJ/lltSHnOEiFGjR9vWqSuZHSLdepI1YTDGJz
dBVR3h2L2sALlHCK7fu//STeJCKhfRg8w2ijU41Jan0EtiAN/0JdjwMfarfa3xVY
NxrstRaLKR2RujXo1kE+ZKmkv6rwaTKRX19lZiFaDuSKUyKLmlwXwy8CgYEAxtqA
VkVup5RCPv0XVAPBgEQymby+XT9VkOZ92l7qN6OGTwLrqB6u0eIxzprRrg+ziBui
gmtph5IeauyXzxgs74kTDTRyFpf7u2D37cgoHgOCzpYWhVLFoKKDAchMBwizf94i
4RTNohnuiznDbXPE6Kwbt9hu+pGmVfJ9MoYohSUCgYEAlzYzLTQMlcMToqoiTWEu
qvB54WsICBz/QXfVbbEocoNpEKMDBh8gYKHHZUPXSsl6448kWKbnoIOC/PNEUf84
ACvbCRqCqQpQ+iYmM7owWySqgfozHnoGnxfiEeLKDOzMZfhqPvOJ+m2bm9oX364w
8VMwqETdBs3iMpvloHBMQLsCgYBlXTY0Q9pfJA0MzLiag/ucJadhhLvJDqLQZR3c
NavQxtOM7SExJsrYhCP9fB/MSYarp+KT9qrph4tn18iesWUeiVIj1gseB0Uzw/89
v1zrx8BjDFDYj1PcSVrcirxujXnqgVJoR9F2gx986un6nhvOuwS6F2Ki6aHPh/dX
zNlU3QKBgQDTN6fEY86U3w9fj8ER1T++bSrjlxnatc3OsPytQka5dF08M/ZYXJev
dAqp/XJFEdoPlawSTVPEu5VDvj10xXCWjlU5S93EKLBp3M+9I76dG3VJk1cLK1tU
kGD7NbBgzNjrhiox+AYQ/qGPoQIH65E9/tDm07BAktIK9Sb2wmRyKg==
-----END RSA PRIVATE KEY-----
)KEY";