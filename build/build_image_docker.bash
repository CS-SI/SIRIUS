#! /bin/bash
# Build the sirius image docker and save it (.tar)
if [ "$#" -ne 1 ]; then
    echo " Build the sirius image docker and save it (.tar)"
    echo " Usage: $0 PROJECT_DIR"
    exit 1
fi

LOCAL_PROJECT_DIR=$1
LOCAL_PROJECT_DIR="$(readlink -f ${LOCAL_PROJECT_DIR})"

CURRENT_DIR=$(pwd)
# TODO: SIRIUS_VERSION ?
cd ${LOCAL_PROJECT_DIR}
SIRIUS_GIT_BRANCH_NAME="$(git rev-parse --abbrev-ref HEAD)"
#DOCKER_VAR_SIRIUS_REVISION_COMMIT="$(git rev-parse HEAD)"
DOCKER_VAR_SIRIUS_REVISION_COMMIT="sirius_revision_commit"

#DOCKER_VAR_SIRIUS_VERSION=${SIRIUS_GIT_BRANCH_NAME}
DOCKER_VAR_SIRIUS_VERSION="0.3.0"

echo " "
echo " * PROJECT_DIR=${LOCAL_PROJECT_DIR}"
echo " "

DOCKER_VAR_INSTALL_DIR="/opt/sirius"
DOCKER_VAR_BUILD_DIR="/tmp/sirius_build_dir"
DOCKER_VAR_CMAKE_BUILD_TYPE=Release

echo " * DOCKER_VAR_SIRIUS_VERSION=${DOCKER_VAR_SIRIUS_VERSION}"
echo " * DOCKER_VAR_SIRIUS_REVISION_COMMIT=${DOCKER_VAR_SIRIUS_REVISION_COMMIT}"
echo " * DOCKER_VAR_INSTALL_DIR=${DOCKER_VAR_INSTALL_DIR}"
echo " * DOCKER_VAR_BUILD_DIR=${DOCKER_VAR_BUILD_DIR}"

# Build sirius docker base
DOCKER_IMAGENAME="sirius.base"
echo " * DOCKER_IMAGENAME=${DOCKER_IMAGENAME}"

# Build docker
# ----------------------
sudo docker build --build-arg http_proxy=$http_proxy --build-arg https_proxy=$https_proxy --build-arg no_proxy=$no_proxy  -t ${DOCKER_IMAGENAME}:latest  -t ${DOCKER_IMAGENAME}:${DOCKER_VAR_SIRIUS_VERSION} -f- . <<EOF
FROM osgeo/gdal:latest
LABEL maintainer="CS SI"

ENV DEBIAN_FRONTEND noninteractive

RUN echo "Europe/Paris" > /etc/timezone

# ------------------------------
# Needed packages for next step
# ------------------------------
RUN apt-get update -y \\
 && apt-get upgrade -y \\
 && apt-get install -y apt-transport-https software-properties-common python3-software-properties

# -----------------
# Standard packages
# -----------------
RUN apt-get update -y \\
 && apt-get install -y --no-install-recommends --allow-unauthenticated \\
    ca-certificates wget \\
    build-essential \\
    gcc \\
    g++-7 \\
    libfftw3-dev \\
    cmake \\
    python3-pip \\
    python3-dev \\
    python3-numpy \\
    git
#    python3-gdal \\
#    libgdal-dev \\
#    gdal-bin \\
# && apt-get autoclean -y \
# && apt-get autoremove -y \
# && rm -rf /var/lib/apt/lists/*

RUN gdalinfo --version

WORKDIR ${DOCKER_VAR_BUILD_DIR}
COPY . ${DOCKER_VAR_BUILD_DIR}

# Usage: ./build_install_sirius.sh PROJECT_DIR INSTALL_DIR SIRIUS_VERSION SIRIUS_REVISION_COMMIT
RUN ./build/build_install_sirius.sh  \\
            ${DOCKER_VAR_BUILD_DIR} \\
            ${DOCKER_VAR_INSTALL_DIR} \\
            "${DOCKER_VAR_SIRIUS_VERSION}" \\
            "${DOCKER_VAR_SIRIUS_REVISION_COMMIT}"
#        && rm -rf ${DOCKER_VAR_BUILD_DIR}


ENV LD_LIBRARY_PATH=${DOCKER_VAR_INSTALL_DIR}/lib:\$LD_LIBRARY_PATH
ENV PATH=${DOCKER_VAR_INSTALL_DIR}/bin:\$PATH

EOF

# Build sirius docker OPER
DOCKER_IMAGENAME_OPER="sirius"
echo " * DOCKER_IMAGENAME_OPER=${DOCKER_IMAGENAME_OPER}"

# Build docker
# ----------------------
sudo docker build --build-arg http_proxy=$http_proxy --build-arg https_proxy=$https_proxy --build-arg no_proxy=$no_proxy  -t ${DOCKER_IMAGENAME_OPER}:latest  -t ${DOCKER_IMAGENAME_OPER}:${DOCKER_VAR_SIRIUS_VERSION} -f- . <<EOF
FROM ${DOCKER_IMAGENAME}:${DOCKER_VAR_SIRIUS_VERSION}
LABEL maintainer="CS SI"

ENTRYPOINT ["${DOCKER_VAR_INSTALL_DIR}/bin/sirius"]

EOF

cd ${CURRENT_DIR}
# Save docker image base
# ----------------------
echo "Save the image docker ${DOCKER_IMAGENAME}_${DOCKER_VAR_SIRIUS_VERSION}.tar..." \
  && sudo docker save ${DOCKER_IMAGENAME}:${DOCKER_VAR_SIRIUS_VERSION} -o ${DOCKER_IMAGENAME}_${DOCKER_VAR_SIRIUS_VERSION}.tar

# Save docker image
# ----------------------
echo "Save the image docker ${DOCKER_IMAGENAME_OPER}_${DOCKER_VAR_SIRIUS_VERSION}.tar..." \
  && sudo docker save ${DOCKER_IMAGENAME_OPER}:${DOCKER_VAR_SIRIUS_VERSION} -o ${DOCKER_IMAGENAME_OPER}_${DOCKER_VAR_SIRIUS_VERSION}.tar
