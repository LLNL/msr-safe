#!/bin/sh

set -o nounset
set -o pipefail

. /etc/sysconfig/msr-safe

start() {
  if [ -z "${WL_CPU:-}" ]; then
    WL_CPU=$(printf 'wl_%.2x%x\n' $(lscpu | grep "CPU family:" | awk -F: '{print $2}') $(lscpu | grep "Model:" | awk -F: '{print $2}'))
  fi

  if [ -z "${WHITELIST:-}" ]; then
    WHITELIST="/usr/share/msr-safe/whitelists/${WL_CPU}" 
  fi

  if [ -f "/usr/share/msr-safe/whitelists/${WL_CPU}" ]; then
    /sbin/modprobe msr-safe && \
    cat "${WHITELIST}" > /dev/cpu/msr_whitelist

    return $?
  else
    return 1
  fi
}

stop() {
    echo > /dev/cpu/msr_whitelist && \
    /sbin/rmmod msr-safe

    return $?
}

rc=0

case "${1:-}" in
  
  start)
      start
      rc=$?
      ;;
  stop)
      stop
      rc=$?
      ;;
  *)
      echo $"Usage: $0 {start|stop}"
      exit 2
esac

exit $rc
