FROM ubuntu:16.04
ENV LC_CTYPE C.UTF-8

RUN apt-get update
RUN apt-get upgrade -y
RUN apt-get install g++ gcc valgrind cmake gdb ssh rsync -y
RUN mkdir /root/.ssh
RUN echo "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQCyjMVU39fACD7oh7egZnr54RWe1YT7tlg1qgW6v+9DEC3JCNvaZTXcQsYmeo+112AqSOOT2P7VzRSLIVHyNSMPZCEi3/gFYwC86qpUHu3lzvFooCnialKQ8NpHCIHGRlGsWNOImgY+aMlCEZlceLAsLBwAI4YYar9jMQdO8d4PLkxPVT5WawV6r2xZH1BEh6LNw6QmKj6MMec0YzIxOvjrWT8D899aAgKulRkQIgXrovZm7NMH88fMzNqljpAz3jaOT6nJ0uS/9m7CRoiry/u1smZJuHmYhCmpU5KxbdGGLLvEvVkZX6SFeFvGpeKxz5hX0zx/l6iKopDBxQGGrHHB jakubkowalski@MacBookPro Jakub" >> ~/.ssh/authorized_keys

ENTRYPOINT service ssh start && /bin/bash

EXPOSE 22