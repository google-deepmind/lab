FROM kaixhin/vnc

MAINTAINER fcarey@gmail.com

RUN apt-get update && apt-get install -y \
    curl \
    zip \
    unzip \
    software-properties-common \
    python-software-properties && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# We need to add a custom PPA to pick up JDK8, since trusty doesn't
# have an openjdk8 backport.  openjdk-r is maintained by a reliable contributor:
# Matthias Klose (https://launchpad.net/~doko).  It will do until
# we either update the base image beyond 14.04 or openjdk-8 is
# finally backported to trusty; see e.g.
#   https://bugs.launchpad.net/trusty-backports/+bug/1368094
RUN add-apt-repository -y ppa:openjdk-r/ppa && \
    apt-get update && \
    apt-get install -y openjdk-8-jdk openjdk-8-jre-headless && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* && \
    which java && \
    java -version && \
    update-ca-certificates -f

# Running bazel inside a `docker build` command causes trouble, cf:
#   https://github.com/bazelbuild/bazel/issues/134
# The easiest solution is to set up a bazelrc file forcing --batch.
# RUN echo "startup --batch" >>/root/.bazelrc
# Similarly, we need to workaround sandboxing issues:
#   https://github.com/bazelbuild/bazel/issues/418
# RUN echo "build --spawn_strategy=standalone --genrule_strategy=standalone" \
#    >>/root/.bazelrc
# ENV BAZELRC /root/.bazelrc
# Install the most recent bazel release.
ENV BAZEL_VERSION 0.4.3
RUN mkdir /bazel && \
    cd /bazel && \
    curl -fSsL -O https://github.com/bazelbuild/bazel/releases/download/$BAZEL_VERSION/bazel-$BAZEL_VERSION-installer-linux-x86_64.sh && \
    curl -fSsL -o /bazel/LICENSE.txt https://raw.githubusercontent.com/bazelbuild/bazel/master/LICENSE.txt && \
    chmod +x bazel-*.sh && \
    ./bazel-$BAZEL_VERSION-installer-linux-x86_64.sh && \
    cd / && \
    rm -f /bazel/bazel-$BAZEL_VERSION-installer-linux-x86_64.sh

# Install deepmind-lab dependencies
RUN apt-get update && apt-get install -y \
  lua5.1 \
  liblua5.1-0-dev \
  libffi-dev \
  gettext \
  freeglut3-dev \
  libsdl2-dev \
  libosmesa6-dev \
  python-dev \
  python-numpy \
  realpath \
  build-essential

#Enable RANDR option for vnc server.
COPY ./vnc.sh /opt/vnc.sh

# Set the default X11 Display.
ENV DISPLAY :1

ENV lab_dir /lab
RUN mkdir /$lab_dir
COPY . /$lab_dir
WORKDIR $lab_dir

RUN bazel build :deepmind_lab.so --define headless=osmesa

# RUN bazel run :python_module_test --define headless=osmesa

#RUN bazel run :random_agent --define headless=false



