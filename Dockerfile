FROM ubuntu:20.04

ENV TZ=UTC \ 
    DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -yqq \
    git \
    gcc \
    g++ \
    cmake \
    libjsoncpp-dev \
    uuid-dev \
    zlib1g-dev \
    openssl \
    libssl-dev \
    postgresql-all \
    doxygen

# Defina o diretório de trabalho para o seu aplicativo
WORKDIR /app

# Clone o repositório Drogon do GitHub
RUN git clone https://github.com/drogonframework/drogon && \
    cd drogon && \
    git submodule update --init


WORKDIR /app/drogon
RUN gcc --version
RUN g++ --version
RUN ls
RUN mkdir build && \
        cd build && \
        cmake --std=c++17 -lstdc++fs -DCMAKE_BUILD_TYPE=Release .. && \
        make && \
        make install

# Copie o seu código fonte para o contêiner
COPY . /app/dev_bank_with_drogon_framework

WORKDIR /app/dev_bank_with_drogon_framework
RUN mkdir build && \
        cd build && \
        cmake --std=c++17 -lstdc++fs -DCMAKE_BUILD_TYPE=Release .. && \
        make

# Exponha a porta em que o aplicativo Drogon será executado
EXPOSE 8080

# Inicie o seu aplicativo Drogon
WORKDIR /app/dev_bank_with_drogon_framework/build/
CMD ["/app/dev_bank_with_drogon_framework/build/DevBankWithDrogonFramework"]
