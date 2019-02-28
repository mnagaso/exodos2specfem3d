# exodos2specfem3d

This repository includes a small code which converts a mesh data of Exodos II format to SPECFEM3D format.  

## to compile 
`docker-compose up -d` and `dokcer exec -it (name of the container generated) bash` to prepare the necessary libraries to compile the codes.  
Then `cd exodos2specfem_IOSS` and `make` will compile the code for conversion.  
Executables will be created in exodos2specfem2\_IOSS/bin/.
