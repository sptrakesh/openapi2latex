FROM sptrakesh/boost AS base

COPY CMakeLists.txt /opt/spt/openapi-latex/
COPY preamble.tex /opt/spt/openapi-latex/
COPY src /opt/spt/openapi-latex/src

WORKDIR /opt/spt/openapi-latex
RUN cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/opt/local -B build -S . \
  && cmake --build build -j8 \
  && cmake --install build

FROM sptrakesh/cppruntime

COPY --from=base /opt/spt/bin /opt/spt/bin

RUN apk add --no-cache cmark \
  && addgroup spt \
  && adduser -DS -g "SPT User" -G spt -s /bin/sh -h /home/spt spt \
  && mkdir -p /opt/spt/logs \
  && chown spt:spt /opt/spt/logs

WORKDIR /opt/spt
ENV LD_LIBRARY_PATH=/usr/lib \
  PATH=$PATH:/opt/spt/bin
USER spt
ENTRYPOINT [ "/opt/spt/bin/oa2tex" ]
