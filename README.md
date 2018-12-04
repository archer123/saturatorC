# saturatorC
Rewrite the saturator of Keith Winstein on C  
Original C++ version : https://github.com/keithw/multisend/tree/master/sender 

To build the saturator, just use  
```bash  
make   
```  
Then we can find a executeable file "saturatr" that we need.  

To lanch the saturatr, first lanch saturatr on the server  
```bash
./saturatr
```
And then lanch the saturatr on the client with client IP, client_name(which should be ens33) and server IP (which the RELIABLE_IP TEST_IP could be the same as client IP)  
```bash
./saturatr <RELIABLE_IP> ens33 <TEST_IP> ens33 <SERVER_IP>
```

