# Используем Ubuntu 22.04 как базовый образ
FROM ubuntu:22.04

# Устанавливаем метаданные
LABEL maintainer="xloned"
LABEL description="MLE/MLS Parameter Estimation with Boost and Python"

# Устанавливаем переменные окружения для неинтерактивной установки
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# Устанавливаем все необходимые зависимости
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libboost-math-dev \
    python3 \
    python3-pip \
    python3-venv \
    && rm -rf /var/lib/apt/lists/*

# Создаем рабочую директорию
WORKDIR /app

# Копируем файлы проекта
COPY include/ ./include/
COPY src/ ./src/
COPY python/ ./python/
COPY input/ ./input/
COPY main.cpp ./
COPY Makefile.docker ./Makefile

# Создаем директорию для вывода
RUN mkdir -p output

# Устанавливаем Python зависимости
RUN python3 -m venv python/venv && \
    python/venv/bin/pip install --no-cache-dir \
    numpy \
    matplotlib \
    scipy

# Компилируем проект
RUN make

# Устанавливаем точку входа
ENTRYPOINT ["./mle_estimator"]

# По умолчанию запускаем программу
CMD []
