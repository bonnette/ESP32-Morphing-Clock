//wifi network
char wifi_ssid[] = "Your SSID";
//wifi password
char wifi_pass[] = "Your WiFi Password";
//timezone
char timezone[5] = "-8" ; // in theory, this is set by the location in openweathermap
//use 24h time format
char military[3] = "N";     // 24 hour mode? Y/N
//use metric data
char u_metric[3] = "N";     // use metric for units? Y/N
//date format
char date_fmt[7] = "M.D.Y"; // date format: D.M.Y or M.D.Y or M.D or D.M or D/M/Y.. looking for trouble
//open weather map api key 
String apiKey   = "mykey"; //e.g a hex string like "c3f3c20cf253315f8646755291334ab0"
//the city you want the weather for 
String location = "Conroe,TX"; //e.g. "Paris,FR"
