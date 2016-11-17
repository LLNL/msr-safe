#!/bin/bash

set -o errexit
set -o nounset
set -o pipefail

DIR=$(dirname $(readlink -f $0))
REV=$(git rev-parse HEAD)
TMPDIR=$(mktemp -d)

tar -C "${DIR}/../.." -zcf "${TMPDIR}/msr-safe-${REV}.tar.gz" --transform s/msr-safe/msr-safe-${REV}/ msr-safe
mv "${TMPDIR}/msr-safe-${REV}.tar.gz" "${DIR}/"
rpmbuild -D "_sourcedir ${DIR}" -ba "${DIR}/msr-safe.spec"
rm "${DIR}/msr-safe-${REV}.tar.gz"
rmdir "${TMPDIR}"
