Repository for LSOccer 
=========
Simulator project:
contains all components of both Client and Server, with custom libraries and Dockerfile

# How-to-use
1. Start by cloning the main repository
2. Once cloned, next step is to use the docker-use "dockerBuild.sh" and "DockerRun.sh" files,
    to build the Image and then run the Container of the server  
3. At this point, go to the client directory, execute "./client 127.0.0.1" and you'll be connected to the Dockerized Server
4. For the match to start, you have to connect 10 clients: 
    - 2 captains that define the Teams
    - 8 other clients that act as team members (4 each)
5. Once the match is over, you can choose to play another match with the same players or close the application  
