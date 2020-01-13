//Incluir librerías a utilizar
#include <esp_adc_cal.h>
#define TINY_GSM_MODEM_SIM800 //Se define modelo de modulo sim
#include <TinyGsmClient.h> //Se incluye libreria TinyGSM para controlar los comandos AT hacia el modulo SIM
#include "ThingsBoard.h" //Se incluye libreria de thingsboard para conectar y enviar los datos via mqtt
#include <ArduinoJson.h> //Se incluye la libreria arduinojson para crear archivos json con los datos a enviar a la plataforma thingsboard
// Librería para la comunicación I2C y la RTClib
#include <Wire.h>
#include "RTClib.h"
#include <string.h>
// Librerías para la tarjeta SD
#include "FS.h"
#include "SD.h"
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>

int bits;
int maximum_bits;
float Irms;
float offset = 120;
int inPinI;
int sampleI;
float sqV,sumV,sqI,sumI,instP,sumP; 
float voltageI;     
esp_adc_cal_characteristics_t *adc_chars = new esp_adc_cal_characteristics_t;
const int sensorPin = 33;   // seleccionar la entrada para el sensor
int sensorValue;         // variable que almacena el valor raw (0 a 1023)
float value;   
// GPIO where the DS18B20 is connected to
// Pin en donde el sensor de temperatura DS18B20 está conectado
const int oneWireBus = 2;
// Setup a oneWire instance to communicate with any OneWire devices
// Configurar una instancia de oneWire para comunicarlo con cualquier otro dispositivo OneWire
OneWire oneWire(oneWireBus);
// Pass our oneWire reference to Dallas Temperature sensor 
// Pasar la referencia de oneWire al sensor de temperatura Dallas Temperature
DallasTemperature sensors(&oneWire);

// Declaramos un RTC DS3231
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
//Asignar datos de la tarjeta sim
  //Asignar dirección compañía de celular
  const char apn[] = "m2m.entel.cl";
  
  //Asignar user
  const char gprsUser[] = "entelpcs";
  
  //Asignar password
  const char gprsPass[] = "entelpcs";
  
  //Asignar PIN
  const char simPIN[] = "";

//inicializar hardware serial monitor
#define SerialMon Serial
#define SerialAT Serial2

//definir la consola serial para imprimir debugs si es necesario
//#define TINY_GSM_DEBUG SerialMon

//inicializar pines hardware serial
#define RXD2 16
#define TXD2 17

//Asignar número de serie dispositivo
const char id_serie[] = "A005";

//Asignar access token thingsboard
#define TOKEN "A1"

//Asignar IP thingsboard
#define THINGSBOARD_SERVER "192.241.142.219"
#define THINGSBOARD_PORT    80

// Initialize GSM modem
// Inicializar modem GSM
TinyGsm modem(SerialAT);

// Initialize GSM client
// Inicializar cliente GSM
TinyGsmClient client(modem);

// Initialize ThingsBoard instance
// Inicializar instancia de Thingsboard
ThingsBoard tb(client);
// Set to true, if modem is connected
// Colocar en True si el modem está conectado
bool modemConnected = false;

// Definir variable de string para el mensaje a enviar a thingsboard
String dataMessage;

// Se define una lookup table para corregir la no linealidad del pin analógico del ESP32
// Esta lookup table está sacada de un comentario y no corresponde exactamente a todos los ESP sin embargo aproxima mejor el comportamiento
// Para mejorar esto hay que mapear el rango para cada dispositivo con un osciloscopio y un generador de señal
int ADC_LUT[4096] = { 0,
0,66,70,74,78,81,82,83,85,86,87,89,90,91,92,94,
95,96,97,98,99,100,101,102,103,104,105,106,106,107,108,109,
110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,
126,127,128,130,131,132,133,134,135,136,137,138,139,140,142,143,
144,145,146,148,149,151,152,153,155,156,158,159,160,161,162,163,
164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,
180,181,182,183,184,185,186,188,189,190,191,192,193,194,195,196,
198,199,200,201,202,203,204,205,207,208,209,210,211,212,213,214,
215,217,218,219,220,221,222,223,224,226,227,228,230,231,232,234,
235,236,237,239,240,241,242,243,244,245,246,247,248,249,250,251,
252,253,254,255,256,257,258,259,260,261,263,264,265,266,267,268,
270,271,272,273,274,275,277,278,279,280,281,282,283,284,286,287,
288,289,290,291,292,293,294,295,296,297,298,299,300,301,302,302,
303,305,306,307,308,309,310,311,313,314,315,316,317,318,319,321,
322,323,324,324,325,326,327,328,329,330,331,332,333,334,335,336,
337,338,339,340,341,342,343,343,344,345,346,347,348,349,350,351,
351,352,354,355,357,358,359,361,362,363,365,366,367,369,370,372,
373,374,376,377,379,380,382,383,384,385,386,387,388,389,390,391,
392,393,394,395,396,397,398,398,399,400,402,403,404,405,407,408,
409,410,412,413,414,416,417,418,419,420,421,422,423,424,425,426,
427,428,428,429,430,431,433,434,435,436,437,439,440,441,442,443,
445,446,447,448,449,450,451,452,453,454,455,456,457,458,459,460,
461,462,463,464,465,466,467,468,470,471,472,473,474,476,477,478,
479,480,481,482,483,484,485,486,487,488,489,490,491,492,493,494,
495,497,499,501,503,506,508,510,512,514,515,516,517,518,520,521,
522,523,525,526,527,528,530,531,532,533,534,535,536,537,539,540,
541,542,543,544,545,546,547,548,550,551,552,553,554,555,556,557,
558,559,560,561,562,563,564,565,567,568,569,570,571,572,573,574,
575,576,577,578,579,581,582,583,584,585,586,587,588,589,590,591,
592,594,595,596,598,599,600,602,603,604,606,607,608,609,610,611,
612,613,614,615,616,617,618,619,620,621,622,623,624,625,626,627,
628,629,630,631,632,633,634,635,636,637,638,639,640,641,642,643,
644,644,645,646,647,648,649,650,650,651,652,653,654,655,656,657,
657,658,659,660,661,662,663,664,665,666,667,668,669,670,671,671,
672,674,675,676,677,678,679,680,682,683,684,685,686,687,688,690,
691,692,693,694,696,697,698,699,701,702,703,704,705,706,708,709,
710,711,712,713,714,715,716,717,718,719,721,722,724,725,727,728,
730,731,733,734,736,737,738,738,739,740,741,742,743,744,744,745,
746,747,748,749,750,751,751,752,753,755,756,757,758,760,761,762,
763,764,766,767,768,769,770,771,772,773,774,776,777,778,779,780,
781,782,783,784,785,786,788,789,790,791,792,793,794,796,797,798,
799,800,801,802,803,805,806,807,808,809,810,811,812,813,814,815,
817,818,819,820,821,822,823,824,825,826,827,828,829,830,831,832,
833,834,836,837,838,840,841,842,844,845,846,848,849,850,851,852,
853,854,855,856,857,858,859,860,861,863,864,865,866,867,869,870,
871,873,874,875,877,878,879,880,881,882,883,884,884,885,886,887,
888,889,889,890,891,892,893,893,894,895,896,897,898,899,901,902,
903,904,906,907,908,909,911,912,913,914,915,916,917,918,919,920,
921,922,923,924,925,926,927,928,929,930,931,933,934,935,936,937,
938,940,941,942,943,944,945,946,947,948,949,950,950,951,952,953,
954,955,956,957,957,958,959,960,961,963,964,965,966,968,969,970,
972,973,974,976,977,978,979,980,981,982,983,984,985,987,988,989,
990,991,992,993,994,996,997,998,999,1000,1001,1002,1004,1005,1006,1007,
1008,1009,1010,1011,1012,1013,1014,1015,1016,1017,1018,1019,1020,1020,1021,1022,
1023,1024,1026,1027,1028,1029,1030,1031,1032,1034,1035,1036,1037,1038,1039,1041,
1042,1043,1044,1045,1047,1048,1049,1050,1051,1052,1054,1055,1056,1057,1058,1059,
1060,1061,1062,1063,1064,1065,1066,1067,1068,1069,1070,1071,1072,1073,1074,1076,
1077,1078,1079,1081,1082,1083,1084,1086,1087,1088,1089,1090,1091,1092,1093,1094,
1095,1096,1097,1098,1099,1100,1101,1102,1103,1104,1105,1106,1107,1108,1110,1111,
1112,1113,1114,1116,1117,1118,1119,1120,1122,1123,1124,1125,1126,1127,1129,1130,
1131,1132,1133,1134,1136,1137,1138,1139,1140,1141,1142,1143,1144,1145,1146,1147,
1148,1149,1150,1151,1152,1153,1154,1155,1156,1157,1158,1159,1160,1161,1162,1163,
1164,1165,1166,1167,1168,1169,1170,1171,1172,1173,1174,1176,1177,1178,1179,1180,
1181,1182,1184,1185,1185,1186,1187,1188,1188,1189,1190,1191,1191,1192,1193,1194,
1194,1195,1196,1197,1197,1198,1199,1200,1201,1202,1204,1205,1206,1208,1209,1211,
1212,1214,1215,1216,1217,1218,1219,1220,1221,1222,1223,1224,1225,1226,1226,1227,
1228,1229,1230,1231,1232,1233,1234,1236,1237,1238,1239,1240,1242,1243,1244,1245,
1246,1248,1249,1250,1251,1252,1252,1253,1254,1255,1256,1257,1258,1259,1260,1261,
1262,1263,1264,1265,1266,1267,1268,1269,1270,1271,1272,1273,1274,1275,1276,1277,
1278,1279,1280,1281,1282,1284,1285,1286,1287,1288,1289,1290,1291,1293,1294,1295,
1296,1297,1298,1299,1299,1300,1301,1302,1303,1304,1305,1306,1307,1308,1308,1309,
1310,1311,1312,1313,1315,1316,1317,1319,1320,1321,1323,1324,1325,1327,1328,1329,
1331,1332,1333,1334,1336,1337,1338,1339,1341,1342,1343,1344,1345,1346,1347,1348,
1349,1350,1351,1352,1353,1354,1355,1356,1357,1358,1359,1360,1361,1362,1363,1364,
1365,1366,1366,1367,1368,1369,1370,1371,1372,1373,1373,1374,1375,1376,1378,1379,
1381,1382,1384,1385,1387,1388,1390,1392,1393,1394,1395,1396,1397,1398,1399,1400,
1401,1402,1403,1404,1405,1406,1407,1408,1409,1410,1411,1413,1414,1415,1416,1417,
1419,1420,1421,1422,1423,1424,1426,1427,1428,1429,1430,1431,1432,1433,1434,1436,
1437,1438,1439,1440,1441,1442,1443,1444,1445,1446,1447,1448,1449,1450,1451,1452,
1453,1454,1455,1456,1457,1459,1460,1461,1462,1463,1464,1465,1466,1467,1468,1469,
1470,1471,1472,1474,1475,1476,1477,1479,1480,1481,1482,1483,1485,1486,1487,1488,
1489,1490,1492,1493,1494,1495,1496,1497,1498,1499,1500,1501,1502,1503,1504,1505,
1506,1507,1508,1509,1510,1510,1511,1512,1513,1514,1515,1516,1517,1518,1519,1520,
1522,1524,1527,1530,1532,1535,1537,1537,1538,1539,1540,1541,1542,1543,1544,1545,
1546,1547,1547,1548,1549,1550,1551,1552,1553,1554,1556,1557,1558,1560,1561,1562,
1563,1565,1566,1567,1568,1569,1570,1571,1572,1573,1574,1575,1576,1577,1578,1579,
1580,1581,1582,1583,1584,1585,1586,1588,1589,1590,1591,1592,1593,1594,1596,1597,
1598,1599,1600,1601,1602,1603,1605,1606,1607,1608,1609,1610,1611,1612,1613,1614,
1615,1616,1618,1619,1620,1621,1622,1623,1625,1626,1627,1628,1629,1630,1632,1633,
1634,1635,1636,1637,1638,1639,1640,1641,1641,1642,1643,1644,1645,1646,1647,1648,
1649,1650,1651,1652,1653,1654,1655,1656,1657,1658,1659,1660,1661,1662,1663,1665,
1666,1667,1668,1670,1671,1672,1673,1674,1676,1677,1678,1679,1680,1681,1682,1683,
1684,1685,1686,1687,1688,1688,1689,1690,1691,1692,1693,1694,1695,1696,1697,1698,
1700,1701,1703,1704,1706,1707,1709,1711,1712,1713,1714,1715,1716,1717,1718,1719,
1720,1721,1722,1723,1724,1725,1726,1727,1728,1729,1731,1732,1733,1734,1735,1736,
1738,1739,1740,1741,1742,1743,1745,1746,1748,1749,1751,1753,1754,1756,1757,1759,
1760,1761,1761,1762,1763,1763,1764,1765,1765,1766,1766,1767,1768,1768,1769,1770,
1770,1771,1772,1772,1773,1773,1774,1775,1775,1776,1777,1778,1779,1780,1781,1782,
1783,1784,1785,1786,1787,1788,1790,1791,1792,1793,1794,1795,1796,1797,1799,1800,
1801,1802,1803,1804,1806,1807,1808,1809,1810,1812,1813,1814,1815,1816,1817,1819,
1820,1821,1822,1823,1825,1826,1827,1828,1829,1830,1831,1832,1834,1835,1836,1837,
1838,1839,1840,1841,1842,1843,1844,1845,1846,1847,1848,1849,1850,1850,1851,1852,
1853,1854,1855,1856,1857,1858,1860,1861,1862,1863,1864,1865,1867,1868,1869,1870,
1871,1873,1874,1875,1876,1877,1878,1880,1881,1882,1883,1884,1885,1886,1888,1889,
1890,1891,1891,1892,1893,1894,1895,1896,1897,1898,1899,1900,1901,1902,1903,1904,
1905,1906,1907,1908,1909,1910,1911,1912,1913,1914,1915,1916,1917,1918,1919,1920,
1921,1922,1923,1924,1924,1925,1926,1927,1928,1929,1930,1931,1932,1933,1934,1935,
1936,1937,1938,1940,1941,1942,1943,1945,1946,1947,1948,1949,1951,1952,1953,1955,
1956,1957,1958,1960,1961,1962,1964,1965,1966,1968,1969,1970,1971,1972,1974,1975,
1976,1977,1978,1980,1981,1982,1983,1984,1985,1986,1987,1989,1990,1991,1992,1993,
1994,1995,1996,1997,1998,1999,2000,2001,2002,2003,2004,2005,2006,2007,2008,2009,
2010,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025,2026,
2027,2028,2029,2030,2030,2031,2032,2033,2033,2034,2034,2034,2035,2035,2036,2036,
2036,2037,2037,2038,2038,2039,2039,2039,2040,2040,2041,2041,2042,2042,2042,2043,
2043,2044,2044,2045,2045,2045,2046,2046,2047,2047,2047,2048,2049,2050,2051,2052,
2053,2054,2055,2056,2057,2058,2059,2060,2061,2062,2063,2064,2065,2066,2068,2069,
2070,2071,2072,2074,2075,2076,2077,2079,2080,2081,2082,2083,2084,2085,2086,2087,
2088,2090,2091,2092,2093,2094,2095,2096,2097,2098,2099,2101,2102,2103,2104,2105,
2106,2107,2108,2109,2111,2112,2113,2114,2115,2116,2118,2119,2120,2121,2122,2123,
2125,2126,2127,2128,2129,2131,2132,2133,2134,2135,2136,2138,2139,2140,2141,2142,
2144,2145,2146,2146,2147,2148,2149,2150,2151,2152,2153,2154,2155,2156,2157,2158,
2159,2160,2161,2162,2163,2164,2165,2166,2167,2167,2168,2169,2170,2171,2172,2173,
2174,2175,2176,2178,2179,2180,2181,2183,2184,2185,2186,2188,2189,2190,2191,2193,
2194,2195,2196,2197,2198,2199,2200,2201,2202,2203,2204,2205,2206,2207,2208,2209,
2210,2212,2213,2214,2215,2216,2217,2218,2219,2220,2222,2223,2224,2225,2226,2227,
2228,2229,2230,2232,2233,2234,2235,2236,2237,2238,2239,2240,2242,2243,2244,2245,
2246,2247,2248,2249,2251,2252,2253,2254,2255,2256,2257,2258,2259,2260,2261,2262,
2263,2264,2265,2266,2267,2268,2269,2270,2271,2272,2273,2274,2275,2276,2277,2278,
2279,2280,2281,2282,2283,2284,2284,2285,2286,2287,2288,2289,2290,2291,2292,2293,
2294,2294,2295,2296,2297,2298,2299,2300,2301,2301,2302,2303,2304,2305,2306,2308,
2309,2310,2311,2312,2313,2315,2316,2317,2318,2319,2320,2322,2323,2324,2326,2327,
2328,2329,2331,2332,2333,2334,2336,2337,2338,2339,2340,2341,2342,2343,2344,2345,
2346,2347,2348,2349,2350,2351,2352,2353,2354,2356,2357,2359,2360,2362,2363,2364,
2366,2367,2369,2370,2370,2371,2372,2373,2374,2375,2376,2377,2378,2379,2380,2381,
2382,2383,2384,2385,2386,2387,2388,2389,2390,2391,2393,2394,2395,2396,2397,2398,
2399,2400,2401,2402,2404,2405,2406,2408,2409,2410,2412,2413,2414,2416,2417,2418,
2418,2419,2420,2421,2422,2423,2424,2425,2426,2427,2428,2429,2429,2430,2431,2432,
2433,2434,2435,2436,2438,2439,2440,2441,2442,2443,2444,2445,2446,2447,2448,2449,
2450,2452,2453,2454,2455,2456,2457,2458,2459,2461,2462,2463,2464,2465,2466,2467,
2468,2470,2471,2472,2473,2474,2475,2476,2478,2479,2480,2481,2482,2483,2484,2485,
2487,2488,2489,2490,2491,2492,2493,2495,2496,2497,2498,2499,2500,2501,2503,2504,
2505,2506,2507,2508,2510,2511,2512,2513,2514,2515,2516,2517,2518,2519,2520,2521,
2522,2523,2524,2525,2526,2527,2528,2529,2531,2532,2533,2535,2536,2538,2539,2541,
2542,2544,2545,2546,2548,2549,2550,2552,2553,2554,2556,2557,2558,2560,2561,2562,
2563,2563,2564,2565,2566,2567,2568,2569,2569,2570,2571,2572,2573,2574,2575,2576,
2577,2578,2580,2581,2583,2584,2586,2587,2589,2590,2592,2593,2594,2595,2596,2597,
2598,2599,2600,2601,2602,2602,2603,2604,2605,2606,2607,2608,2610,2611,2613,2614,
2616,2617,2619,2620,2622,2623,2624,2625,2626,2627,2628,2629,2630,2631,2632,2633,
2634,2635,2636,2637,2638,2639,2640,2641,2642,2643,2644,2646,2647,2648,2649,2651,
2652,2653,2654,2656,2657,2658,2659,2660,2661,2662,2663,2664,2665,2666,2667,2668,
2669,2670,2671,2672,2673,2674,2675,2677,2678,2679,2680,2681,2682,2683,2684,2685,
2686,2687,2688,2689,2691,2692,2693,2694,2695,2696,2697,2698,2700,2701,2702,2703,
2704,2705,2706,2707,2708,2709,2711,2712,2713,2714,2715,2716,2717,2718,2719,2720,
2721,2722,2723,2724,2725,2726,2728,2729,2730,2731,2732,2733,2734,2735,2736,2737,
2738,2739,2739,2740,2741,2742,2743,2744,2745,2746,2747,2748,2748,2749,2750,2751,
2752,2753,2755,2756,2757,2758,2760,2761,2762,2763,2764,2766,2767,2768,2770,2771,
2772,2774,2775,2777,2778,2779,2781,2782,2784,2785,2786,2787,2788,2789,2790,2791,
2792,2793,2794,2795,2796,2797,2798,2799,2800,2801,2802,2803,2804,2805,2805,2806,
2807,2808,2809,2810,2811,2812,2813,2814,2815,2816,2817,2818,2819,2820,2822,2823,
2824,2825,2827,2828,2829,2830,2831,2833,2835,2837,2840,2842,2844,2846,2848,2849,
2850,2850,2851,2852,2852,2853,2854,2854,2855,2856,2856,2857,2858,2859,2859,2860,
2861,2861,2862,2863,2863,2864,2865,2866,2867,2867,2868,2869,2870,2871,2872,2872,
2873,2874,2875,2876,2877,2877,2878,2879,2880,2881,2882,2883,2884,2885,2887,2888,
2889,2890,2891,2892,2893,2894,2895,2896,2897,2898,2899,2900,2901,2902,2903,2904,
2905,2906,2907,2908,2909,2910,2911,2912,2914,2915,2916,2918,2919,2920,2922,2923,
2924,2925,2927,2928,2929,2930,2931,2932,2934,2935,2936,2937,2938,2939,2940,2941,
2942,2944,2945,2945,2946,2947,2948,2949,2950,2950,2951,2952,2953,2954,2955,2956,
2956,2957,2958,2959,2960,2961,2962,2963,2964,2965,2966,2967,2968,2969,2971,2972,
2973,2974,2975,2976,2977,2978,2980,2981,2982,2984,2985,2986,2988,2989,2990,2992,
2993,2994,2995,2996,2997,2998,2999,3000,3000,3001,3002,3003,3004,3005,3006,3007,
3008,3009,3010,3011,3012,3013,3014,3015,3016,3017,3018,3019,3020,3020,3021,3022,
3023,3024,3025,3027,3028,3029,3030,3031,3032,3033,3035,3036,3037,3038,3039,3040,
3041,3042,3043,3044,3045,3046,3047,3048,3049,3050,3051,3052,3053,3054,3055,3057,
3058,3059,3061,3062,3064,3065,3066,3068,3069,3071,3072,3073,3074,3075,3075,3076,
3077,3078,3079,3080,3080,3081,3082,3083,3084,3085,3085,3086,3087,3088,3089,3090,
3092,3093,3095,3096,3097,3099,3100,3101,3103,3104,3105,3106,3107,3108,3110,3111,
3112,3113,3114,3115,3116,3117,3118,3120,3121,3122,3123,3124,3125,3126,3128,3129,
3130,3131,3132,3133,3134,3136,3137,3137,3138,3139,3140,3141,3142,3142,3143,3144,
3145,3146,3146,3147,3148,3149,3150,3151,3151,3152,3153,3155,3156,3157,3158,3159,
3160,3161,3162,3163,3164,3166,3167,3168,3169,3170,3171,3172,3173,3174,3175,3176,
3177,3178,3179,3180,3181,3182,3183,3184,3185,3186,3187,3188,3189,3190,3190,3191,
3192,3193,3194,3195,3196,3197,3198,3199,3200,3201,3202,3202,3203,3204,3205,3206,
3207,3208,3208,3209,3210,3211,3212,3213,3214,3215,3215,3216,3218,3219,3220,3222,
3223,3224,3225,3227,3228,3229,3231,3232,3233,3234,3235,3236,3236,3237,3238,3239,
3240,3241,3242,3243,3243,3244,3245,3246,3247,3248,3249,3250,3252,3253,3254,3255,
3257,3258,3259,3261,3262,3263,3264,3265,3266,3266,3267,3268,3269,3269,3270,3271,
3272,3272,3273,3274,3275,3275,3276,3277,3278,3278,3279,3280,3281,3282,3283,3284,
3285,3286,3287,3288,3289,3290,3291,3292,3293,3294,3295,3296,3297,3299,3300,3301,
3302,3303,3304,3306,3307,3308,3309,3310,3311,3312,3313,3314,3315,3316,3317,3318,
3318,3319,3320,3321,3322,3323,3324,3325,3325,3326,3327,3328,3329,3330,3331,3332,
3332,3333,3334,3335,3336,3337,3338,3339,3339,3340,3341,3342,3343,3344,3345,3346,
3348,3349,3350,3352,3353,3354,3356,3357,3358,3360,3361,3361,3362,3362,3363,3364,
3364,3365,3366,3366,3367,3367,3368,3369,3369,3370,3371,3371,3372,3373,3373,3374,
3374,3375,3376,3376,3377,3378,3379,3380,3381,3382,3383,3384,3385,3386,3387,3388,
3389,3390,3391,3392,3393,3393,3394,3395,3396,3397,3398,3398,3399,3400,3401,3402,
3403,3403,3404,3405,3406,3407,3408,3409,3410,3411,3412,3413,3414,3415,3416,3417,
3418,3419,3420,3421,3422,3423,3424,3425,3427,3428,3429,3431,3432,3434,3435,3437,
3438,3439,3440,3441,3442,3442,3443,3443,3444,3444,3445,3445,3446,3447,3447,3448,
3448,3449,3449,3450,3450,3451,3452,3452,3453,3453,3454,3454,3455,3455,3456,3457,
3458,3459,3460,3461,3462,3463,3464,3465,3466,3467,3468,3469,3470,3471,3472,3473,
3474,3474,3475,3476,3477,3478,3479,3480,3481,3482,3482,3483,3484,3485,3486,3487,
3488,3489,3490,3491,3492,3493,3494,3495,3496,3497,3498,3499,3500,3501,3502,3503,
3504,3505,3506,3506,3507,3508,3509,3510,3511,3511,3512,3513,3514,3515,3516,3516,
3517,3518,3519,3520,3520,3521,3522,3522,3523,3524,3524,3525,3526,3527,3527,3528,
3529,3529,3530,3531,3531,3532,3533,3533,3534,3535,3535,3536,3537,3538,3538,3539,
3540,3541,3541,3542,3543,3544,3545,3545,3546,3547,3548,3549,3549,3550,3551,3552,
3553,3553,3554,3555,3556,3557,3558,3558,3559,3560,3561,3562,3563,3564,3564,3565,
3566,3567,3568,3569,3570,3572,3573,3575,3576,3577,3579,3580,3582,3583,3584,3585,
3586,3587,3587,3588,3589,3590,3591,3591,3592,3593,3594,3595,3595,3596,3597,3598,
3598,3599,3600,3601,3602,3602,3603,3604,3604,3605,3606,3607,3607,3608,3609,3610,
3610,3611,3612,3613,3613,3614,3615,3616,3616,3617,3618,3619,3620,3621,3622,3623,
3624,3625,3626,3627,3628,3629,3630,3631,3632,3632,3633,3634,3635,3635,3636,3637,
3637,3638,3639,3639,3640,3641,3641,3642,3643,3643,3644,3645,3645,3646,3647,3647,
3648,3649,3649,3650,3651,3651,3652,3653,3653,3654,3655,3655,3656,3657,3657,3658,
3658,3659,3660,3660,3661,3662,3662,3663,3664,3664,3665,3666,3666,3667,3668,3669,
3669,3670,3671,3671,3672,3673,3674,3674,3675,3676,3676,3677,3678,3679,3679,3680,
3681,3682,3683,3683,3684,3685,3686,3687,3688,3688,3689,3690,3691,3692,3693,3694,
3694,3695,3696,3697,3697,3698,3699,3699,3700,3701,3701,3702,3703,3703,3704,3705,
3705,3706,3707,3707,3708,3709,3709,3710,3711,3711,3712,3713,3714,3714,3715,3716,
3717,3718,3719,3720,3720,3721,3722,3723,3724,3725,3726,3726,3727,3728,3729,3729,
3730,3731,3731,3732,3732,3733,3734,3734,3735,3736,3736,3737,3737,3738,3739,3739,
3740,3741,3741,3742,3742,3743,3744,3744,3745,3745,3746,3747,3747,3748,3748,3749,
3749,3750,3751,3751,3752,3752,3753,3753,3754,3755,3755,3756,3756,3757,3758,3758,
3759,3759,3760,3761,3762,3763,3764,3766,3767,3768,3769,3771,3772,3773,3774,3776,
3776,3777,3777,3778,3778,3779,3779,3779,3780,3780,3781,3781,3782,3782,3782,3783,
3783,3784,3784,3785,3785,3785,3786,3786,3787,3787,3788,3788,3789,3789,3789,3790,
3790,3791,3791,3792,3792,3793,3793,3794,3795,3795,3796,3797,3797,3798,3798,3799,
3800,3800,3801,3802,3802,3803,3804,3804,3805,3806,3806,3807,3808,3808,3809,3810,
3810,3811,3812,3812,3813,3814,3814,3815,3816,3816,3817,3818,3818,3819,3820,3820,
3821,3822,3822,3823,3824,3824,3825,3825,3826,3827,3827,3828,3828,3829,3830,3830,
3831,3831,3832,3832,3833,3834,3834,3835,3835,3836,3836,3837,3838,3838,3839,3839,
3840,3841,3841,3842,3842,3843,3843,3844,3845,3845,3846,3846,3847,3848,3848,3849,
3849,3850,3851,3851,3852,3852,3853,3853,3854,3855,3855,3856,3856,3857,3858,3859,
3859,3860,3861,3861,3862,3863,3863,3864,3865,3866,3866,3867,3868,3868,3869,3870,
3871,3871,3872,3873,3873,3874,3874,3875,3876,3876,3877,3878,3878,3879,3879,3880,
3881,3881,3882,3883,3883,3884,3884,3885,3886,3886,3887,3887,3888,3889,3889,3890,
3891,3892,3892,3893,3894,3894,3895,3896,3896,3897,3898,3898,3899,3900,3900,3901,
3902,3903,3903,3904,3905,3905,3906,3906,3907,3907,3908,3909,3909,3910,3910,3911,
3911,3912,3913,3913,3914,3914,3915,3916,3916,3917,3917,3918,3918,3919,3920,3920,
3921,3921,3922,3923,3923,3924,3924,3925,3926,3926,3927,3927,3928,3929,3929,3930,
3930,3931,3932,3932,3933,3933,3934,3935,3935,3936,3936,3937,3937,3938,3939,3939,
3940,3940,3941,3941,3942,3942,3943,3943,3944,3944,3945,3946,3946,3947,3947,3948,
3948,3949,3949,3950,3950,3951,3951,3952,3953,3953,3954,3954,3955,3955,3956,3956,
3957,3958,3958,3959,3959,3960,3960,3961,3962,3962,3963,3963,3964,3964,3965,3966,
3966,3967,3967,3968,3968,3969,3970,3970,3971,3971,3972,3972,3973,3974,3974,3975,
3975,3976,3976,3977,3978,3978,3979,3979,3980,3980,3981,3982,3982,3983,3983,3984,
3984,3985,3986,3986,3987,3987,3988,3989,3989,3990,3990,3991,3992,3992,3993,3993,
3994,3995,3995,3996,3996,3997,3998,3998,3999,3999,4000,4000,4001,4002,4002,4003,
4003,4004,4004,4005,4006,4006,4007,4007,4008,4008,4009,4010,4010,4011,4011,4012,
4012,4013,4014,4014,4015,4015,4016,4016,4017,4018,4018,4019,4019,4020,4020,4021,
4021,4022,4023,4023,4024,4024,4025,4025,4026,4027,4027,4028,4028,4029,4029,4030,
4031,4031,4032,4032,4033,4033,4034,4035,4035,4036,4036,4037,4037,4038,4038,4039,
4040,4040,4041,4041,4042,4042,4043,4043,4044,4045,4045,4046,4046,4047,4047,4048,
4049,4049,4050,4050,4051,4051,4052,4052,4053,4053,4054,4054,4055,4055,4056,4057,
4057,4058,4058,4059,4059,4060,4060,4061,4061,4062,4062,4063,4063,4064,4065,4066,
4067,4068,4069,4070,4070,4071,4072,4073,4074,4075,4076,4077,4078,4079,4080
} ;

void setup() {

  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_0);
  // put your setup code here, to run once:
  //inicializar puerto serial ESP32
  SerialMon.begin(115200);
  delay(5000);
  SerialMon.println("Monitor serial inicializado");
  //Inicializar puerto serie SIM800L
  SerialAT.begin(115200);
  delay(3000);
  pinMode(4,OUTPUT);
  digitalWrite(4, HIGH);   // set the RTS off
  delay(1000);
  digitalWrite(4, LOW);   // set the RTS on
  //inicializar RTC
  if (! rtc.begin()) {
 Serial.println("No hay un módulo RTC");
 //while (1);
 }
 //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
 
  //inicializar sensor de temperatura
sensors.begin();

  //inicializar sensor de voltaje DC
// no se inicializa ya que es un pin analógico que sólo se activa al leer el voltaje en el pin
  //inicializar sensor de corriente
// no se inicializa ya que es un pin analógico que solo se activa al leer el voltaje en el pin
  //inicializar SD
// Initialize SD card
// Se inicializa la tarjeta SD
  SD.begin(); 
// si SD.begin() entrega un valor 0 entonces significa que falló y se imprime en el puerto serial
  if(!SD.begin()) {  
    Serial.println("Card Mount Failed");
    //return;
  }
  uint8_t cardType = SD.cardType(); //se extrae variable del tipo de tarjeta, si entrega None, significa que no hay una tarjeta SD
  if(cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    //return;
  }
  // otra vez se chequea si la tarjeta SD está activa
  Serial.println("Initializing SD card...");
  if (!SD.begin()) {
    Serial.println("ERROR - SD card initialization failed!");
    //return;    // init failed
  }
  // If the data.txt file doesn't exist
  // SI el documento data.txt no existe
  // Create a file on the SD card and write the data labels
  // crear el documento en la tarjeta SD y escribir el encabezado
  File file = SD.open("/data.txt");
  if(!file) {
    Serial.println("File doens't exist");
    Serial.println("Creating file...");
    writeFile(SD, "/data.txt", "json_data \r\n");
  }
  else {
    Serial.println("File already exists"); // si detecta el archivo simplemente lo cierra 
  }
  file.close();
  //inicializar MODEM
  modem.init(); //inicializa el modem
  String modemInfo = modem.getModemInfo(); //solicita la información del modem
  //imprime la información del modem (colocar ejemplo de output)
  SerialMon.print(F("Modem: ")); 
  SerialMon.println(modemInfo);
}

void loop() {
  delay(59000); //delay de 1 segundo antes de ejecutar el resto del código, al final del código es un delay de 59 seg para en resultado 
  //crear un loop que lea los datos cada 1 minuto y los envíe por internet a la plataforma
  // put your main code here, to run repeatedly:
//código para testear los comandos AT desde la consola serial
  //Read SIM800 output (if available) and print it in Arduino IDE Serial Monitor
//  if(SerialAT.available()){
//    SerialMon.write(SerialAT.read());
//  }
  //Read Arduino IDE Serial Monitor inputs (if available) and send them to SIM800
//  if(SerialMon.available()){    
  //  SerialAT.write(SerialMon.read());
  //}
  
  
  //leer hora actual
  DateTime now = rtc.now();
  SerialMon.println(now.unixtime());
  Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
  //leer sensor de temperatura
sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);
  float temperatureF = sensors.getTempFByIndex(0);
  Serial.print(temperatureC);
  Serial.println("ºC");
  Serial.print(temperatureF);
  Serial.println("ºF");
  //leer sensor de corriente
  //Leer corriente pin 34
  //double Irms3 = calcIrms(1480, 34);
  float Irms3 = get_corriente_3();
  SerialMon.print("Corriente AC: ");
  SerialMon.println(Irms3);
  //Leer corriente pin 35
  double Irms2 = calcIrms(1480, 35);
  //Leer corriente pin 32
  double Irms1 = calcIrms(1480, 32);
//falta añadir el código aquí!!!!!
  //leer sensor de voltaje DC
   sensorValue = analogRead(sensorPin);          // realizar la lectura
   sensorValue = (int)ADC_LUT[sensorValue];
   value = fmap(sensorValue, 0, 4095, 0.0, 15.8);   // cambiar escala a 0.0 - 25.0
  SerialMon.print("voltaje DC bits: ");
  SerialMon.println(sensorValue);
  SerialMon.print("voltaje DC esc: ");
  SerialMon.println(value);
  //guardar json en la SD
const size_t capacity = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3)+50; //se crea un documento json a enviar
DynamicJsonDocument doc(capacity);
String var1 =String(now.unixtime());  //Enviar solo horario UTC, thingsboard lo adaptará al horario del usuario
String var2 ="000";
String combinedString = "";
combinedString = var1 + var2;

doc["ts"] = combinedString;

JsonObject values = doc.createNestedObject("values");
values["T05"] = temperatureC;
values["V05"] = value;
values["C05"] = Irms3;
char output[100];
serializeJson(doc, output);
//SD.begin();
char result[120];   // array to hold the result.
strcpy(result,output); // copy string one into the result.
strcat(result,"\r\n"); // append string two to the result.
appendFile(SD, "/data.txt", result);
//appendFile(SD, "/data.txt", "ejemplo\r\n");
  //leer stack de datos
//Si el modem está conectado intentar conectarse a la red gsrm
  if (!modemConnected) {
    SerialMon.print(F("Waiting for network..."));
    if (!modem.waitForNetwork()) {
        SerialMon.println(" fail");
        delay(1000);
        return;
    }
    SerialMon.println(" OK");

    SerialMon.print(F("Connecting to "));
    // Se conecta al sistema de internet de la sim card
    SerialMon.print(apn);
    //realiza la conección y verifica el tema
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) { //debería crear un loop que intente unas 5 veces y continue con el resto del código
        SerialMon.println(" fail");
        delay(1000);
        return;
    }

    modemConnected = true;
    SerialMon.println(" OK");
  }
  //intentar enviar los datos a la plataforma
  if (!tb.connected()) {
    // Connect to the ThingsBoard
    SerialMon.print("Connecting to: ");
    SerialMon.print(THINGSBOARD_SERVER);
    SerialMon.print(" with token ");
    SerialMon.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      SerialMon.println("Failed to connect");
      return; //Se podría intentar un número de intentos antes de salir del void loop()
    }
  }

  SerialMon.println("Sending data...");

  // Uploads new telemetry to ThingsBoard using MQTT. 
  // Se envía los datos de telemetría a Thingsboard usando MQTT.
  // See https://thingsboard.io/docs/reference/mqtt-api/#telemetry-upload-api 

  tb.sendTelemetryJson(output);
  //tb.sendTelemetryFloat("temperatura", random(50,400)/10.0);
  //tb.sendTelemetryFloat("voltajeDC", random(100, 140)/10.0);
  //tb.sendTelemetryFloat("corrienteAC", random(100, 1000)/10.0);

  tb.loop();
}
// Write to the SD card (DON'T MODIFY THIS FUNCTION)
void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}

float fmap(float x, float in_min, float in_max, float out_min, float out_max)
{
   return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
//Funciónes para medir corriente
double calcIrms(unsigned int Number_of_Samples, int inPinI)
{
  //int SupplyVoltage=3300;
  for (unsigned int n = 0; n < Number_of_Samples; n++)
  {
    sampleI = analogRead(inPinI);
    //Serial.print(sampleI);
    sampleI = (int)ADC_LUT[sampleI];
    //Serial.print(sampleI);
    voltageI = map(sampleI, 0, 4095, 0, 3300);
    //Serial.print("Medicion en bits: ");
    //Serial.println(sampleI);
    // Digital low pass filter extracts the 2.5 V or 1.65 V dc offset,
    //  then subtract this - signal is now centered on 0 counts.
    //offsetI = (offsetI + (voltageI-offsetI)/4096);
    //Serial.print("offsetI: ");
    //Serial.print(offsetI);
    //filteredI = voltageI - offsetI;
   //Serial.print("VoltageI: ");
   //Serial.println(voltageI);
    // Root-mean-square method current
    // 1) square current values
    sqI = voltageI * voltageI;
    //sqI = filteredI * filteredI;
    // 2) sum
    sumI += sqI;
  }

  //double I_RATIO = ICAL *((SupplyVoltage/1000.0) / (ADC_COUNTS));
  float I_RATIO = 111.1*(1.1/4096);
  float Irms = I_RATIO * sqrt(sumI / Number_of_Samples);

  //Reset accumulators
  sumI = 0;
  //--------------------------------------------------------------------------------------

  return Irms;
}

float get_corriente_3()
{
  int conversion;
  float voltajeSensor;
  float corriente=0;
  float Sumatoria=0;
  long tiempo=millis();
  int N=0;
  while(millis()-tiempo<600)//Duración 0.5 segundos(Aprox. 30 ciclos de 60Hz)
  { 
    conversion = analogRead(34);
    //conversion = (int)ADC_LUT[conversion];
    //voltajeSensor = conversion * (1.1 / 4095);////voltaje del sensor
    //corriente=voltajeSensor*111.1; //corriente=VoltajeSensor*(30A/1V)
    corriente = conversion;
    Sumatoria=Sumatoria+sq(corriente);//Sumatoria de Cuadrados
    N=N+1;
    delay(1);
  }
  Sumatoria=Sumatoria*2;//Para compensar los cuadrados de los semiciclos negativos.
  corriente=sqrt((Sumatoria)/N); //ecuación del RMS
  if (corriente > 0) {
    corriente = corriente + offset;
  }
  corriente = corriente * (3.3 / 4095) * 100;
  return(corriente);
}

float get_corriente_2()
{
  int conversion;
  float voltajeSensor;
  float corriente=0;
  float Sumatoria=0;
  long tiempo=millis();
  int N=0;
  while(millis()-tiempo<600)//Duración 0.5 segundos(Aprox. 30 ciclos de 60Hz)
  { 
    conversion = analogRead(35);
    //conversion = (int)ADC_LUT[conversion];
    //voltajeSensor = conversion * (1.1 / 4095);////voltaje del sensor
    //corriente=voltajeSensor*111.1; //corriente=VoltajeSensor*(30A/1V)
    corriente = conversion;
    Sumatoria=Sumatoria+sq(corriente);//Sumatoria de Cuadrados
    N=N+1;
    delay(1);
  }
  Sumatoria=Sumatoria*2;//Para compensar los cuadrados de los semiciclos negativos.
  corriente=sqrt((Sumatoria)/N); //ecuación del RMS
  if (corriente > 0) {
    corriente = corriente + offset;
  }
  corriente = corriente * (3.3 / 4095) * 100;
  return(corriente);
}
float get_corriente_1()
{
  int conversion;
  float voltajeSensor;
  float corriente=0;
  float Sumatoria=0;
  long tiempo=millis();
  int N=0;
  while(millis()-tiempo<600)//Duración 0.5 segundos(Aprox. 30 ciclos de 60Hz)
  { 
    conversion = analogRead(32);
    //conversion = (int)ADC_LUT[conversion];
    //voltajeSensor = conversion * (1.1 / 4095);////voltaje del sensor
    //corriente=voltajeSensor*111.1; //corriente=VoltajeSensor*(30A/1V)
    corriente = conversion;
    Sumatoria=Sumatoria+sq(corriente);//Sumatoria de Cuadrados
    N=N+1;
    delay(1);
  }
  Sumatoria=Sumatoria*2;//Para compensar los cuadrados de los semiciclos negativos.
  corriente=sqrt((Sumatoria)/N); //ecuación del RMS
  if (corriente > 0) {
    corriente = corriente + offset;
  }
  corriente = corriente * (3.3 / 4095) * 100;
  return(corriente);
}
