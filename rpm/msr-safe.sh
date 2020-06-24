#!/bin/sh

set -o nounset
set -o pipefail

. /etc/sysconfig/msr-safe

al_cpu() {
  printf 'al_%.2x%x\n' \
  $(grep -m1 'cpu family' /proc/cpuinfo | cut -f2 -d: | tr -d ' ') \
  $(grep -m1 'model' /proc/cpuinfo | cut -f2 -d: | tr -d ' ')
}

start() {
  if [ -z "${AL_CPU:-}" ]; then
    AL_CPU=$(al_cpu)
  fi

  if [ -z "${APPROVED_LIST:-}" ]; then
    APPROVED_LIST="/usr/share/msr-safe/approved_lists/${AL_CPU}"
  fi

  if [ -f "/usr/share/msr-safe/approved_lists/${AL_CPU}" ]; then
    /sbin/modprobe msr-safe && \
    cat "${APPROVED_LIST}" > /dev/cpu/msr_approved_list

    return $?
  else
    return 1
  fi
}

stop() {
    echo > /dev/cpu/msr_approved_list && \
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
