FROM mkaichen/ubuntu-setup:bazel
ENV APP_DIR /usr/src/testify
WORKDIR $APP_DIR
COPY . $APP_DIR

RUN apt-get update && apt-get install -y git\
    python-setuptools python-dev build-essential
RUN easy_install pip
RUN pip install futures grpc protobuf numpy
RUN python -m pip install grpcio

CMD ["make"]
