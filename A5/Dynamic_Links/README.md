# Lab 5 OSPF Routing Algorithm

## Instructions to run the code

* Makefile is provided to compile the code with given commands: 
```
make ospf : compiles the code and makes directory Outputs
make clean : removes the executable ospf and gives new empty Outputs directory
```

* If you want to compile the code manually, use the following command:
```
g++ ospf_dyn.cpp -o ospf -g -O3
mkdir Outputs
```

* Please make sure to __create the Outputs directory__ before running the code.

* Run the code as follows:
```
./ospf <input_file>
```
* The program will ask to enter a command. The possible commands are listed below :
    * Exit program and kill all routers
    ```
    q
    ```
    * Remove link between two nodes
    ```
    r <id1> <id2>
    ```
    * Adding link between two nodes
    ```
    a <id1> <id2> <min_weigtht> <max_weight>
    ```
