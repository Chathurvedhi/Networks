import os
import sys
import socket

def client(nr_ip, port_num):
    clientsocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    while(True):
        hostname = input("Enter Server name: ")
        #hi
        msg = hostname.encode()
        if(hostname == "bye"):
            clientsocket.sendto(msg,(nr_ip, port_num+53))
            exit()
        clientsocket.sendto(msg,(nr_ip, port_num+53))
        ip_info = clientsocket.recvfrom(1024)
        ip_info = ip_info[0].decode()
        print("Final DNS Mapping: ", ip_info)

def nr_server(nr_ip,rds_ip, port_num):
    nr_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    nr_socket.bind((nr_ip, port_num+53))
    while(True):
        data, client_ip = nr_socket.recvfrom(1024)      #data is hostname
        if data.decode() == "bye":                      #send bye to rds and exit
            nr_socket.sendto(data, (rds_ip, port_num+54))
            exit()
        nr_socket.sendto(data, (rds_ip, port_num+54))   #send hostname to rds
        tld_info, rds_ip1 = nr_socket.recvfrom(1024)    #tld_info is tld_ip and tld_port
        tld_info = tld_info.decode()
        tld_ip, tld_port = tld_info.split()
        nr_socket.sendto(data, (tld_ip, int(tld_port))) #send hostname to tld
        ads_info, tld_ip1 = nr_socket.recvfrom(1024)    #ads_info is ads_ip and ads_port
        ads_info = ads_info.decode()
        ads_ip, ads_port = ads_info.split()
        nr_socket.sendto(data, (ads_ip, int(ads_port))) #send hostname to ads
        ip_info, ads_ip1 = nr_socket.recvfrom(1024)     #ip_info is final DNS mapping
        nr_socket.sendto(ip_info, client_ip)            #send ip_info to client

def rds_server(rds_ip, tld_com, tld_edu, port_num):
    rds_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    rds_socket.bind((rds_ip, port_num+54))
    while(True):
        data, nr_ip = rds_socket.recvfrom(1024)         #data is hostname
        if data.decode() == "bye":                      #send bye to tld and exit
            rds_socket.sendto(data, (tld_com, port_num+55))
            rds_socket.sendto(data, (tld_edu, port_num+56))
            exit()
        if data.decode().endswith(".com"):
            msg = tld_com + " " + str(port_num+55)
            rds_socket.sendto(msg.encode(), nr_ip)  #send tld_com to nr
        elif data.decode().endswith(".edu"):
            msg = tld_edu + " " + str(port_num+56)
            rds_socket.sendto(msg.encode(), nr_ip)  #send tld_edu to nr
        
def tld_com_server(tld_com, ads1, ads1_org, ads2, ads2_org, ads3, ads3_org, port_num):
    tld_com_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    tld_com_socket.bind((tld_com, port_num+55))
    while(True):
        data, nr_ip = tld_com_socket.recvfrom(1024)
        if(data.decode() == "bye"):                     #send bye to ads and exit
            tld_com_socket.sendto(data, (ads1, port_num+57))
            tld_com_socket.sendto(data, (ads2, port_num+58))
            tld_com_socket.sendto(data, (ads3, port_num+59))
            exit()
        data = data.decode()
        data = data.split(".")
        if data[1] == ads1_org:
            msg = ads1 + " " +str(port_num+57)
            tld_com_socket.sendto(msg.encode(), nr_ip)
        elif data[1] == ads2_org:
            msg = ads2 + " " + str(port_num+58)
            tld_com_socket.sendto(msg.encode(), nr_ip)
        elif data[1] == ads3_org:
            msg = ads3 + " " + str(port_num+59)
            tld_com_socket.sendto(msg.encode(), nr_ip)
        else:
            print("Error in tld_com_server")

def tld_edu_server(tld_edu, ads4, ads4_org, ads5, ads5_org, ads6, ads6_org, port_num):
    tld_edu_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    tld_edu_socket.bind((tld_edu, port_num+56))
    while(True):
        data, nr_ip = tld_edu_socket.recvfrom(1024)
        if(data.decode() == "bye"):                     #send bye to ads and exit
            tld_edu_socket.sendto(data, (ads4, port_num+60))
            tld_edu_socket.sendto(data, (ads5, port_num+61))
            tld_edu_socket.sendto(data, (ads6, port_num+62))
            exit()
        data = data.decode()
        data = data.split(".")
        if data[1] == ads4_org:
            msg = ads4 + " " + str(port_num+60)
            tld_edu_socket.sendto(msg.encode(), nr_ip)
        elif data[1] == ads5_org:
            msg = ads5 + " " + str(port_num+61)
            tld_edu_socket.sendto(msg.encode(), nr_ip)
        elif data[1] == ads6_org:
            msg = ads6 + " " + str(port_num+62)
            tld_edu_socket.sendto(msg.encode(), nr_ip)
        else:
            print("Error in tld_edu_server")

def ads1_server(ads1, port_num,ads1_map):
    ads1_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    ads1_socket.bind((ads1, port_num+57))
    while(True):
        data, nr_ip = ads1_socket.recvfrom(1024)
        if(data.decode() == "bye"):                     #exit
            exit()
        data = data.decode()
        #Compare hostname with ads1_map and send ip_info to nr
        if data in ads1_map:
            ads1_socket.sendto(ads1_map[data].encode(), nr_ip)
        else:
            ads1_socket.sendto("Not Found ADS1".encode(), nr_ip)

def ads2_server(ads2, port_num,ads2_map):
    ads2_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    ads2_socket.bind((ads2, port_num+58))
    while(True):
        data, nr_ip = ads2_socket.recvfrom(1024)
        if(data.decode() == "bye"):                     #exit
            exit()
        data = data.decode()
        #Compare hostname with ads2_map and send ip_info to nr
        if data in ads2_map:
            ads2_socket.sendto(ads2_map[data].encode(), nr_ip)
        else:
            ads2_socket.sendto("Not Found ADS2".encode(), nr_ip)
        
def ads3_server(ads3, port_num,ads3_map):
    ads3_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    ads3_socket.bind((ads3, port_num+59))
    while(True):
        data, nr_ip = ads3_socket.recvfrom(1024)
        if(data.decode() == "bye"):                     #exit
            exit()
        data = data.decode()
        #Compare hostname with ads3_map and send ip_info to nr
        if data in ads3_map:
            ads3_socket.sendto(ads3_map[data].encode(), nr_ip)
        else:
            ads3_socket.sendto("Not Found ADS3".encode(), nr_ip)

def ads4_server(ads4, port_num,ads4_map):
    ads4_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    ads4_socket.bind((ads4, port_num+60))
    while(True):
        data, nr_ip = ads4_socket.recvfrom(1024)
        if(data.decode() == "bye"):                     #exit
            exit()
        data = data.decode()
        #Compare hostname with ads4_map and send ip_info to nr
        if data in ads4_map:
            ads4_socket.sendto(ads4_map[data].encode(), nr_ip)
        else:
            ads4_socket.sendto("Not Found ADS4".encode(), nr_ip)

def ads5_server(ads5, port_num,ads5_map):
    ads5_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    ads5_socket.bind((ads5, port_num+61))
    while(True):
        data, nr_ip = ads5_socket.recvfrom(1024)
        if(data.decode() == "bye"):                     #exit
            exit()
        data = data.decode()
        #Compare hostname with ads5_map and send ip_info to nr
        if data in ads5_map:
            ads5_socket.sendto(ads5_map[data].encode(), nr_ip)
        else:
            ads5_socket.sendto("Not Found ADS5".encode(), nr_ip)

def ads6_server(ads6, port_num,ads6_map):
    ads6_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    ads6_socket.bind((ads6, port_num+62))
    while(True):
        data, nr_ip = ads6_socket.recvfrom(1024)
        if(data.decode() == "bye"):                     #exit
            exit()
        data = data.decode()
        #Compare hostname with ads6_map and send ip_info to nr
        if data in ads6_map:
            ads6_socket.sendto(ads6_map[data].encode(), nr_ip)
        else:
            ads6_socket.sendto("Not Found ADS6".encode(), nr_ip)

#Reading file
port_num = sys.argv[1]
port_num = int(port_num)
filename = sys.argv[2]
filename = str(filename)
f = open(filename, "r")
data = f.read()
data = data.split("\n")
f.close()

#IP addresses
nr_ip = (data[1].split())[1]
rds_ip = (data[2].split())[1]
tld_com = (data[3].split())[1]
tld_edu = (data[4].split())[1]
ads1 = (data[5].split())[1]
ads2 = (data[6].split())[1]
ads3 = (data[7].split())[1]
ads4 = (data[8].split())[1]
ads5 = (data[9].split())[1]
ads6 = (data[10].split())[1]

#Maps of hostname 
ads1_map = {}
for i in range(12,17):
    host, ip = data[i].split()
    ads1_map[host] = ip
ads2_map = {}
for i in range(18,23):
    host, ip = data[i].split()
    ads2_map[host] = ip
ads3_map = {}
for i in range(24,29):
    host, ip = data[i].split()
    ads3_map[host] = ip
ads4_map = {}
for i in range(30,35):
    host, ip = data[i].split()
    ads4_map[host] = ip
ads5_map = {}
for i in range(36,41):
    host, ip = data[i].split()
    ads5_map[host] = ip
ads6_map = {}
for i in range(42,47):
    host, ip = data[i].split()
    ads6_map[host] = ip

#Organizations
ads1_org = data[12].split('.')[1]
ads2_org = data[18].split('.')[1]
ads3_org = data[24].split('.')[1]
ads4_org = data[30].split('.')[1]
ads5_org = data[36].split('.')[1]
ads6_org = data[42].split('.')[1]

#Fork calls for each server
for i in range(0,10):
    pid = os.fork()
    if pid == 0:
        if i == 0:
            nr_server(nr_ip, rds_ip, port_num)
        elif i == 1:
            rds_server(rds_ip, tld_com, tld_edu, port_num)
        elif i == 2:
            tld_com_server(tld_com, ads1, ads1_org, ads2, ads2_org, ads3, ads3_org, port_num)
        elif i == 3:
            tld_edu_server(tld_edu, ads4, ads4_org, ads5, ads5_org, ads6, ads6_org, port_num)
        elif i == 4:
            ads1_server(ads1, port_num, ads1_map)
        elif i == 5:
            ads2_server(ads2, port_num, ads2_map)
        elif i == 6:
            ads3_server(ads3, port_num, ads3_map)
        elif i == 7:
            ads4_server(ads4, port_num, ads4_map)
        elif i == 8:
            ads5_server(ads5, port_num, ads5_map)
        elif i == 9:
            ads6_server(ads6, port_num, ads6_map)
client(nr_ip, port_num)

