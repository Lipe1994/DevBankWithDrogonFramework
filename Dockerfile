# Use a imagem base do Alpine Linux
FROM alpine:latest

RUN apk add --no-cache build-base

# Atualize os pacotes e instale as dependências necessárias
RUN apk --no-cache add \
    cmake \
    g++ \
    git \
    jsoncpp-dev \
    sqlite-dev \
    openssl \
    openssl-dev \
    zlib-dev \
    util-linux-dev \
    libpq-dev

# Defina o diretório de trabalho para o seu aplicativo
WORKDIR /app

# Clone o repositório Drogon do GitHub
RUN git clone https://github.com/drogonframework/drogon && \
    cd drogon && \
    git submodule update --init


WORKDIR /app/drogon
RUN mkdir build && \
        cd build && \
        cmake .. && \
        make && \
        make install

# Copie o seu código fonte para o contêiner
COPY . /app/dev_bank_with_drogon_framework

WORKDIR /app/dev_bank_with_drogon_framework
RUN mkdir build && \
        cd build && \
        cmake .. && \
        make

# Exponha a porta em que o aplicativo Drogon será executado
EXPOSE 9999

# Inicie o seu aplicativo Drogon
WORKDIR /app/dev_bank_with_drogon_framework/build/
CMD ["/app/dev_bank_with_drogon_framework/build/DevBankWithDrogonFramework"]
