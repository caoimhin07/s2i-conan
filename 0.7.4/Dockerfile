FROM openshift/base-centos7

# This image provides a conan.io environment you can use to run your C++
# applications.

LABEL maintainer="Pascal Bach <pascal.bach@nextrem.ch>"

EXPOSE 8080

ENV CONAN_VERSION=0.15.0

LABEL io.k8s.description="Platform for building and running conan.io enabled C++ applications" \
      io.k8s.display-name="conan.io ${CONAN_VERSION}" \
      io.openshift.expose-services="8080:http" \
      io.openshift.tags="builder,conan.io"

# CentOS 7 reached EOL June 2024; mirrors are dead.
# Point base, SCL, and EPEL repos at vault/archive servers.
RUN sed -i 's|mirrorlist=http://mirrorlist.centos.org|#mirrorlist=http://mirrorlist.centos.org|g' /etc/yum.repos.d/CentOS-*.repo && \
    sed -i 's|#baseurl=http://mirror.centos.org|baseurl=http://vault.centos.org|g' /etc/yum.repos.d/CentOS-*.repo && \
    yum install -y centos-release-scl epel-release && \
    sed -i 's|mirrorlist=http://mirrorlist.centos.org|#mirrorlist=http://mirrorlist.centos.org|g' /etc/yum.repos.d/*.repo && \
    sed -i 's|# baseurl=http://mirror.centos.org|baseurl=http://vault.centos.org|g' /etc/yum.repos.d/*.repo && \
    sed -i 's|#baseurl=http://mirror.centos.org|baseurl=http://vault.centos.org|g' /etc/yum.repos.d/*.repo && \
    sed -i 's|metalink=https://mirrors.fedoraproject.org|#metalink=https://mirrors.fedoraproject.org|g' /etc/yum.repos.d/epel*.repo && \
    sed -i 's|#baseurl=http://download.fedoraproject.org/pub/epel|baseurl=https://archives.fedoraproject.org/pub/archive/epel|g' /etc/yum.repos.d/epel*.repo && \
    INSTALL_PKGS="python2-pip cmake make gcc gcc-c++ kernel-devel" && \
    yum install -y --setopt=tsflags=nodocs $INSTALL_PKGS && \
    rpm -V $INSTALL_PKGS && \
    yum clean all -y && \
    pip install --upgrade "pip<21" && \
    pip install "conan==$CONAN_VERSION" && \
    conan

# Copy the S2I scripts from the specific language image to $STI_SCRIPTS_PATH
COPY ./s2i/bin/ $STI_SCRIPTS_PATH

# Each language image can have 'contrib' a directory with extra files needed to
# run and build the applications.
#COPY ./contrib/ /opt/app-root

# Drop the root user and make the content of /opt/app-root owned by user 1001
RUN chown -R 1001:0 /opt/app-root
USER 1001

# Set the default CMD to print the usage of the language image
CMD ["sh", "-c", "$STI_SCRIPTS_PATH/usage"]
