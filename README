# ISA 2015 - 6in4 tunnel project
> Jakub Faber (xfaber02)


The 6in4 traffic is sent over the IPv4 Internet inside IPv4 packets.

Project files:
```
.
├── listen4.c
├── listen4.h
├── listen6.c
├── listen6.h
├── logger.c
├── logger.h
├── main.c
├── main.h
├── Makefile
├── manual.pdf
├── README
├── README.html
├── send4.c
├── send4.h
├── send6.c
└── send6.h
```

## Compile
```sh
make
```

## Run as root (need permissions to bind raw sockets)
```sh
./sixtunnel --lan eth0 --wan eth1 --remote 192.168.0.11 --log output.log
```


* `--lan` interface to IPv6 network
* `--wan` interface to IPv4 network
* `--remote` IPv4 address of the other end of tunnel
* `--log` logging file

### Limits
The 6in4 is unable to tunnel packets larger than `MTU_SIZE` (default 1500 bytes)

___

### Example output
```
DateTime |  SourceIP |  DestinationIP | Protocol | SrcPort | DstPort | Time
```
```
2015/11/22 19:06:27 faaa::1 fbbb::4 tcp 46465 9999 -
2015/11/22 19:06:50 faaa::1 fbbb::4 icmp - - -
2015/11/22 19:06:51 faaa::1 fbbb::4 icmp - - -
2015/11/22 19:06:52 faaa::1 fbbb::4 icmp - - -
2015/11/22 19:09:10 fbbb::4 faaa::1 udp 41755 15000 -
2015/11/22 19:09:26 fbbb::4 faaa::1 udp 60190 6679 -
2015/11/22 19:09:36 fbbb::4 faaa::1 icmp - - -
2015/11/22 19:09:37 fbbb::4 faaa::1 icmp - - -
```


### Example Setup

```
PC1 (eth0) <--ipv6--> (eth0) PC2 (eth1) <--ipv4--> (eth0) PC3 (eth1) <--ipv6--> (eth0) PC4
```

## PC1
> /etc/network/interfaces
```
auto eth0
iface eth0 inet6 static
  address faaa::1
  gateway faaa::2
  netmask 64
```

## PC2
> /etc/network/interfaces
```
auto eth1
iface eth1 inet6 static
  address faaa::2
  netmask 64
auto eth0
iface eth0 inet static
  address 192.168.0.2
  netmask 255.255.255.0
  gateway 192.168.0.0
```

```
sudo ip -6 route add default dev eth1
```

```
sudo ./sixtunnel --lan eth0 --wan eth1 --remote 192.168.0.3 --log out2.log
```

## PC3
> /etc/network/interfaces
```
auto eth1
iface eth1 inet6 static
  address fbbb::3
  netmask 64
auto eth0
iface eth0 inet static
  address 192.168.0.3
  netmask 255.255.255.0
  gateway 192.168.0.0
```

```
sudo ip -6 route add default dev eth0
```
```
sudo ./sixtunnel --lan eth1 --wan eth0 --remote 192.168.0.2 --log out3.log
```

## PC4
> /etc/network/interfaces
```
auto eth0
iface eth0 inet6 static
  address fbbb::4
  gateway fbbb::3
  netmask 64
```
