#!/usr/bin/bash
# Bash completion script for grm (Group & Telegram Manager CLI)

_grm_completions() {
  local cur prev words cword
  _init_completion || return

  local global_opts="-h --help -V --version -v --verbose -d --debug -q --quiet -c --config -T --test-dc -F --format --color --no-color"

  if [[ ${cword} -eq 1 ]]; then
    if [[ "${cur}" == -* ]]; then
      COMPREPLY=($(compgen -W "${global_opts}" -- "${cur}"))
    else
      COMPREPLY=($(compgen -W "${commands}" -- "${cur}"))
    fi
    return 0
  fi

  local command="${words[1]}"

  case "${command}" in
    login)
      COMPREPLY=($(compgen -W "-p --phone -k --code -h --help" -- "${cur}"))
      ;;
    chat)
      if [[ ${cword} -eq 2 ]]; then
        COMPREPLY=($(compgen -W "ls -h --help" -- "${cur}"))
      fi
      ;;
    msg)
      if [[ ${cword} -eq 2 ]]; then
        COMPREPLY=($(compgen -W "ls export search -h --help" -- "${cur}"))
      elif [[ ${cword} -eq 4 && "${words[2]}" == "export" ]]; then
        COMPREPLY=($(compgen -W "csv json" -- "${cur}"))
      fi
      ;;
    extract)
      if [[ ${cword} -eq 2 ]]; then
        COMPREPLY=($(compgen -W "bday -h --help" -- "${cur}"))
      fi
      ;;
    topic)
      if [[ ${cword} -eq 2 ]]; then
        COMPREPLY=($(compgen -W "ls -h --help" -- "${cur}"))
      fi
      ;;
    send)
      if [[ ${cword} -eq 2 ]]; then
        if [[ "${cur}" == -* ]]; then
          COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
        else
          COMPREPLY=($(compgen -W "file" -- "${cur}"))
        fi
      elif [[ "${words[2]}" == "file" ]]; then
        COMPREPLY=($(compgen -W "-C --caption -t --topic -h --help" -- "${cur}"))
      fi
      ;;
    *)
      ;;
  esac

  return 0
}

complete -F _grm_completions grm
