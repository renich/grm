#!/usr/bin/bash
# Bash completion script for grm (Group & Telegram Manager CLI)

_grm_completions() {
  local cur prev words cword
  _init_completion || return

  local global_opts="-h --help -V --version -v --verbose -d --debug -q --quiet -c --config -T --test-dc -F --format --color --no-color"
  local commands="login chat msg topic file"

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
        COMPREPLY=($(compgen -W "ls create info set-title set-desc pin unpin delete -h --help" -- "${cur}"))
      fi
      ;;
    msg)
      if [[ ${cword} -eq 2 ]]; then
        COMPREPLY=($(compgen -W "ls export search send info edit delete -h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "ls" ]]; then
        COMPREPLY=($(compgen -W "-n --limit -t --topic -h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "export" ]]; then
        COMPREPLY=($(compgen -W "-f --format -o --output -t --topic -n --limit csv json -h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "search" ]]; then
        COMPREPLY=($(compgen -W "-q --query -n --limit -t --topic -h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "send" ]]; then
        COMPREPLY=($(compgen -W "-a --attach -m --media -C --caption -t --topic -h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "delete" ]]; then
        COMPREPLY=($(compgen -W "--for-everyone -e -h --help" -- "${cur}"))
      fi
      ;;
    topic)
      if [[ ${cword} -eq 2 ]]; then
        COMPREPLY=($(compgen -W "ls create info edit close reopen pin unpin delete -h --help" -- "${cur}"))
      fi
      ;;
    file)
      if [[ ${cword} -eq 2 ]]; then
        COMPREPLY=($(compgen -W "get download-all -h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "get" ]]; then
        COMPREPLY=($(compgen -W "-o --output -t --topic -h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "download-all" ]]; then
        COMPREPLY=($(compgen -W "-o --output -t --topic -n --limit --type photo video doc audio all -h --help" -- "${cur}"))
      fi
      ;;
    *)
      ;;
  esac

  return 0
}

complete -F _grm_completions grm
