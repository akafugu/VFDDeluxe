FROM ubuntu:14.04

# Set up Arduino IDE
#####################

RUN apt-get update \
	&& apt-get install -y wget openjdk-7-jre xz-utils g++ gcc libc6-dev make pkg-config \
	&& apt-get clean \
	&& rm -rf /var/lib/apt/lists/*

ENV ARDUINO_IDE_VERSION 1.8.2
RUN (wget -q -O- https://downloads.arduino.cc/arduino-${ARDUINO_IDE_VERSION}-linux64.tar.xz \
	| tar xJC /usr/local/share \
	&& ln -s /usr/local/share/arduino-${ARDUINO_IDE_VERSION} /usr/local/share/arduino \
	&& ln -s /usr/local/share/arduino-${ARDUINO_IDE_VERSION}/arduino /usr/local/bin/arduino)

# Set up golang
################

ENV GOLANG_VERSION 1.8.3

RUN set -eux; \
	\
# this "case" statement is generated via "update.sh"
	dpkgArch="$(dpkg --print-architecture)"; \
	case "${dpkgArch##*-}" in \
		ppc64el) goRelArch='linux-ppc64le'; goRelSha256='e5fb00adfc7291e657f1f3d31c09e74890b5328e6f991a3f395ca72a8c4dc0b3' ;; \
		i386) goRelArch='linux-386'; goRelSha256='ff4895eb68fb1daaec41c540602e8bb4c1e8bb2f0e7017367171913fc9995ed2' ;; \
		s390x) goRelArch='linux-s390x'; goRelSha256='e2ec3e7c293701b57ca1f32b37977ac9968f57b3df034f2cc2d531e80671e6c8' ;; \
		armhf) goRelArch='linux-armv6l'; goRelSha256='3c30a3e24736ca776fc6314e5092fb8584bd3a4a2c2fa7307ae779ba2735e668' ;; \
		amd64) goRelArch='linux-amd64'; goRelSha256='1862f4c3d3907e59b04a757cfda0ea7aa9ef39274af99a784f5be843c80c6772' ;; \
		*) goRelArch='src'; goRelSha256='5f5dea2447e7dcfdc50fa6b94c512e58bfba5673c039259fd843f68829d99fa6'; \
			echo >&2; echo >&2 "warning: current architecture ($dpkgArch) does not have a corresponding Go binary release; will be building from source"; echo >&2 ;; \
	esac; \
	\
	url="https://golang.org/dl/go${GOLANG_VERSION}.${goRelArch}.tar.gz"; \
	wget -O go.tgz "$url"; \
	echo "${goRelSha256} *go.tgz" | sha256sum -c -; \
	tar -C /usr/local -xzf go.tgz; \
	rm go.tgz; \
	\
	if [ "$goRelArch" = 'src' ]; then \
		echo >&2; \
		echo >&2 'error: UNIMPLEMENTED'; \
		echo >&2 'TODO install golang-any from jessie-backports for GOROOT_BOOTSTRAP (and uninstall after build)'; \
		echo >&2; \
		exit 1; \
	fi; \
	\
	export PATH="/usr/local/go/bin:$PATH"; \
	go version

ENV GOPATH /go
ENV PATH $GOPATH/bin:/usr/local/go/bin:$PATH

RUN mkdir -p "$GOPATH/src" "$GOPATH/bin" && chmod -R 777 "$GOPATH"
WORKDIR $GOPATH

# Set up Arduino Builder
#########################

WORKDIR /build/
RUN apt-get update \
	&& apt-get install -y git
RUN git clone https://github.com/arduino/arduino-builder.git

ENV GOPATH=/build/arduino-builder:$GOPATH
RUN go get github.com/go-errors/errors
RUN go get github.com/stretchr/testify
RUN go get github.com/jstemmer/go-junit-report

WORKDIR /build/arduino-builder
RUN go build arduino.cc/arduino-builder
RUN cp arduino-builder /usr/local/bin

WORKDIR /build/akafugu-libraries
RUN git clone https://github.com/akafugu/WireRtcLibrary.git

WORKDIR /build/VFDDeluxe
COPY . .

RUN mkdir -p /build/output
RUN mkdir pu /builds
RUN arduino-builder -hardware /usr/local/share/arduino/hardware \
	-tools /usr/local/share/arduino/hardware/tools/avr \
	-tools /usr/local/share/arduino/tools-builder \
	-libraries /usr/local/share/arduino/libraries \
	-libraries /build/akafugu-libraries \
	-build-path /build/output \
	-fqbn arduino:avr:leonardo VFDDeluxe.ino

RUN cp /build/output/VFDDeluxe.ino.* /builds/
