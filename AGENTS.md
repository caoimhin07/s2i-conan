# AGENTS.md

## Cursor Cloud specific instructions

### Overview

This is an S2I (Source-to-Image) builder image for C++ applications using Conan.io, targeting CentOS 7/RHEL 7.
Primary commands are `make build VERSION=0.7.4` and `make test VERSION=0.7.4`. See `README.md` for full details.

### Known Infrastructure Issues (CentOS 7 EOL)

CentOS 7 reached end-of-life in June 2024. The following workarounds are baked into the Dockerfile and scripts:

1. **Docker Image Manifest v2 Schema 1**: The `openshift/base-centos7` base image uses the deprecated schema 1 manifest format, which Docker 25+ no longer supports. Before building, pull and convert it with skopeo:
   ```bash
   skopeo copy --format v2s2 docker://docker.io/openshift/base-centos7:latest docker-daemon:openshift/base-centos7:latest
   ```
   Then patch the repos inside a container and commit (see below).

2. **Dead yum mirrors**: `mirrorlist.centos.org` no longer resolves. The Dockerfile handles this by rewriting repos to use `vault.centos.org` and `archives.fedoraproject.org/pub/archive/epel`. However, the **base image itself** must also be patched before building, since BuildKit resolves `FROM` before running `RUN`:
   ```bash
   docker run --name patch-base openshift/base-centos7 bash -c '
     sed -i "s|mirrorlist=http://mirrorlist.centos.org|#mirrorlist=http://mirrorlist.centos.org|g" /etc/yum.repos.d/CentOS-*.repo
     sed -i "s|#baseurl=http://mirror.centos.org|baseurl=http://vault.centos.org|g" /etc/yum.repos.d/CentOS-*.repo
     yum install -y centos-release-scl epel-release
     sed -i "s|mirrorlist=http://mirrorlist.centos.org|#mirrorlist=http://mirrorlist.centos.org|g" /etc/yum.repos.d/*.repo
     sed -i "s|# baseurl=http://mirror.centos.org|baseurl=http://vault.centos.org|g" /etc/yum.repos.d/*.repo
     sed -i "s|#baseurl=http://mirror.centos.org|baseurl=http://vault.centos.org|g" /etc/yum.repos.d/*.repo
     sed -i "s|metalink=https://mirrors.fedoraproject.org|#metalink=https://mirrors.fedoraproject.org|g" /etc/yum.repos.d/epel*.repo
     sed -i "s|#baseurl=http://download.fedoraproject.org/pub/epel|baseurl=https://archives.fedoraproject.org/pub/archive/epel|g" /etc/yum.repos.d/epel*.repo
   '
   docker commit patch-base openshift/base-centos7:latest
   docker rm patch-base
   ```

3. **Conan version**: Conan 0.7.4 was removed from PyPI; the Dockerfile now uses 0.15.0 (oldest available).

4. **Conan remote**: The old `server.conan.io` remote is defunct. The test app was rewritten as a self-contained POSIX HTTP server to avoid depending on external conan packages.

### Required System Dependencies

- **Docker**: Build and test are entirely Docker-based.
- **S2I** (source-to-image v1.4.0+): Install from [GitHub releases](https://github.com/openshift/source-to-image/releases).
- **skopeo**: Needed to pull the base image with manifest format conversion.
- **Make** and **Git**: Standard build tools, pre-installed.

### Building and Testing

```bash
# 1. Pull and convert base image (required once)
skopeo copy --format v2s2 docker://docker.io/openshift/base-centos7:latest docker-daemon:openshift/base-centos7:latest

# 2. Patch base image repos (required once, see step 2 above)

# 3. Build the builder image
SKIP_SQUASH=1 make build VERSION=0.7.4

# 4. Build and run the full test suite
SKIP_SQUASH=1 make test VERSION=0.7.4
```

`SKIP_SQUASH=1` is required because the docker-scripts squash tool is incompatible with modern Docker.

### No Lint/Unit Tests

This project has no linter configuration or automated unit tests. The only testing is the S2I integration test (`make test`) which requires a fully built Docker image.
