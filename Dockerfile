FROM mkaichen/ubuntu-setup:bazel
ENV APP_DIR /usr/src/testify
WORKDIR $APP_DIR

RUN apt-get update && apt-get install -y git
RUN pip install futures protobuf numpy grpcio grpcio-tools

COPY . $APP_DIR
CMD ["make"]
