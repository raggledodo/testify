FROM mkaichen/ubuntu-setup:bazel
ENV APP_DIR /usr/src/testify
WORKDIR $APP_DIR
COPY . $APP_DIR
RUN bazel build --spawn_strategy=standalone //...
RUN cp -Lr bazel-bin bin
RUN tar -czvf testify.tar.gz bin

FROM raggledodo/buildtools:testify
ENV APP_DIR /usr/src/testify
WORKDIR $APP_DIR
COPY --from=0 /usr/src/testify/testify.tar.gz .
RUN tar -xvf testify.tar.gz

RUN echo '#!/usr/bin/env bash\n\
./bin/testify_cpp/test --gtest_break_on_failure --gtest_shuffle --gtest_repeat=25\n\
valgrind --tool=memcheck ./bin/testify_cpp/test --gtest_break_on_failure --gtest_shuffle --gtest_repeat=25\n\
./bin/testify_py/graphast_test\n\
./bin/testify_py/tfgen_test' > docker_test.sh

CMD [ "bash", "docker_test.sh" ]
