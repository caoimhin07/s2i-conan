# AGENTS.md

## Cursor Cloud specific instructions

### Overview

This is an S2I (Source-to-Image) builder image for C++ applications using Conan.io, targeting CentOS 7/RHEL 7.  
Primary commands are `make build VERSION=0.7.4` and `make test VERSION=0.7.4`. See `README.md` for full details.

### Known Infrastructure Issues (CentOS 7 EOL)

CentOS 7 reached end-of-life in June 2024. The following workarounds are required to build:

1. **Docker Image Manifest v2 Schema 1**: The `openshift/base-centos7` base image uses the deprecated schema 1 manifest format, which Docker 25+ no longer supports. Use `skopeo copy --format v2s2 docker://docker.io/openshift/base-centos7:latest docker-daemon:openshift/base-centos7:latest` to pull and convert it.

2. **Dead yum mirrors**: `mirrorlist.centos.org` no longer resolves. After pulling the base image with skopeo, patch yum repos inside a container and commit:
   - Replace `mirrorlist=http://mirrorlist.centos.org` with `baseurl=http://vault.centos.org`
   - Also install `centos-release-scl` inside the container (to create SCL repo files), then patch those SCL repos the same way.
   - For EPEL: use `baseurl=https://archives.fedoraproject.org/pub/archive/epel`.

3. **`python-pip` package name**: The RPM package is `python2-pip` (provides `python-pip`), but `rpm -V python-pip` fails because it checks exact package name. The Dockerfile's `rpm -V` step will fail on this.

4. **Conan 0.7.4 removed from PyPI**: The oldest available version is `0.15.0`.

5. **Conan remote (`server.conan.io`)**: The old Conan remote is defunct. The test app (Poco HTTP time server) cannot resolve its `Poco/1.6.1@lasote/stable` dependency.

Due to items 3-5, `make build` / `make test` cannot complete successfully without Dockerfile modifications. To demonstrate the pipeline, build the image manually (see above workarounds) and run S2I commands directly.

### Required System Dependencies

- **Docker**: Build and test are entirely Docker-based.
- **S2I** (source-to-image v1.4.0+): Install from [GitHub releases](https://github.com/openshift/source-to-image/releases). Note: the `--force-pull` flag was renamed to `--pull-policy` in v1.4.0.
- **skopeo**: Needed to pull the base image with manifest format conversion.
- **Make** and **Git**: Standard build tools, pre-installed.

### Running the S2I Pipeline Manually

```bash
# 1. Pull and convert base image
skopeo copy --format v2s2 docker://docker.io/openshift/base-centos7:latest docker-daemon:openshift/base-centos7:latest

# 2. Patch repos, install packages, and build the builder image (see workarounds above)

# 3. Test S2I usage
s2i usage --pull-policy=never openshift/conan-074-centos7-candidate

# 4. S2I build test app
cd 0.7.4/test/test-app && git init && git config user.email "build@localhost" && git config user.name "builder" && git add -A && git commit -m "Sample commit" && cd -
s2i build --pull-policy=never file://$(pwd)/0.7.4/test/test-app openshift/conan-074-centos7-candidate openshift/conan-074-centos7-candidate-testapp
```

### No Lint/Unit Tests

This project has no linter configuration or automated unit tests. The only testing is the S2I integration test (`make test`) which requires a fully built Docker image.
