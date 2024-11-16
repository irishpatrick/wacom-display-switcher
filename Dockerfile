FROM ubuntu:24.04 AS base

RUN apt update ; apt install -y build-essential cmake ninja-build libgtk-4-dev libx11-dev libxrandr-dev libwacom-dev

FROM base AS build

WORKDIR /app
COPY . .
RUN cmake -B ./build -GNinja -DCMAKE_BUILD_TYPE=Release -DWINDOW_SYSTEM=x11 -DAPP_Version=todo -DCMAKE_INSTALL_PREFIX=/artifacts/x11
RUN cmake --build ./build --config Release ; cmake --install ./build

FROM build AS pack

RUN strip $(find /artifacts -type f -executable) ; tar cf /release.tar /artifacts
