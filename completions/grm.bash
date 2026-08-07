#!/usr/bin/bash
# Modern Bash 5+ completion script for grm (Group & Telegram Manager CLI)
# Requires Bash >= 4.4 (uses compgen -V COMPREPLY and compopt for zero subshells)

_grm_completions() {
  local cur prev words cword
  COMPREPLY=()
  cur="${COMP_WORDS[COMP_CWORD]}"
  prev="${COMP_WORDS[COMP_CWORD-1]}"
  words=("${COMP_WORDS[@]}")
  cword=${COMP_CWORD}

  local global_opts="-h --help -H --help=all -V --version -v --verbose -d --debug -q --quiet -c --config -T --test-dc -F --format -p -P --pretty --color --no-color"
  local commands="login chat msg topic file completion"

  # Completing top-level command or global option
  if [[ ${cword} -eq 1 ]]; then
    if [[ "${cur}" == -* ]]; then
      compgen -V COMPREPLY -W "${global_opts}" -- "${cur}"
    else
      compgen -V COMPREPLY -W "${commands}" -- "${cur}"
    fi
    return 0
  fi

  local cmd="${words[1]}"

  case "${cmd}" in
    login)
      compgen -V COMPREPLY -W "-p --phone -k --code -h --help" -- "${cur}"
      ;;
    chat)
      if [[ ${cword} -eq 2 ]]; then
        if [[ "${cur}" == -* ]]; then
          compgen -V COMPREPLY -W "-h --help" -- "${cur}"
        else
          compgen -V COMPREPLY -W "ls create info set-title set-desc pin unpin delete" -- "${cur}"
        fi
      elif [[ "${words[2]}" == "ls" ]]; then
        compgen -V COMPREPLY -W "-n --limit -S --since -f --filter -h --help" -- "${cur}"
      elif [[ "${words[2]}" == "create" ]]; then
        if [[ ${cword} -eq 3 ]]; then
          compgen -V COMPREPLY -W "group channel" -- "${cur}"
        else
          compgen -V COMPREPLY -W "--private --public -h --help" -- "${cur}"
        fi
      else
        compgen -V COMPREPLY -W "-h --help" -- "${cur}"
      fi
      ;;
    msg)
      if [[ ${cword} -eq 2 ]]; then
        if [[ "${cur}" == -* ]]; then
          compgen -V COMPREPLY -W "-h --help" -- "${cur}"
        else
          compgen -V COMPREPLY -W "ls export search send info edit pin unpin delete" -- "${cur}"
        fi
      elif [[ "${words[2]}" == "ls" ]]; then
        compgen -V COMPREPLY -W "-n --limit -t --topic -S --since -f --filter --sender -r --reverse -h --help" -- "${cur}"
      elif [[ "${words[2]}" == "export" ]]; then
        if [[ "${prev}" == "-f" || "${prev}" == "--format" ]]; then
          compgen -V COMPREPLY -W "csv json" -- "${cur}"
        elif [[ "${prev}" == "-o" || "${prev}" == "--output" ]]; then
          compopt -o filenames
          compgen -V COMPREPLY -f -- "${cur}"
        else
          compgen -V COMPREPLY -W "-f --format -o --output -t --topic -n --limit -h --help" -- "${cur}"
        fi
      elif [[ "${words[2]}" == "search" ]]; then
        compgen -V COMPREPLY -W "-q --query -n --limit -t --topic -S --since -f --filter --sender -h --help" -- "${cur}"
      elif [[ "${words[2]}" == "send" ]]; then
        if [[ "${prev}" == "-a" || "${prev}" == "--attach" ]]; then
          compopt -o filenames
          compgen -V COMPREPLY -f -- "${cur}"
        else
          compgen -V COMPREPLY -W "-a --attach -m --media -C --caption -t --topic -h --help" -- "${cur}"
        fi
      elif [[ "${words[2]}" == "edit" || "${words[2]}" == "info" || "${words[2]}" == "pin" ]]; then
        compgen -V COMPREPLY -W "-t --topic -h --help" -- "${cur}"
      elif [[ "${words[2]}" == "unpin" ]]; then
        compgen -V COMPREPLY -W "-a --all -h --help" -- "${cur}"
      elif [[ "${words[2]}" == "delete" ]]; then
        compgen -V COMPREPLY -W "--for-everyone -e -h --help" -- "${cur}"
      else
        compgen -V COMPREPLY -W "-h --help" -- "${cur}"
      fi
      ;;
    topic)
      if [[ ${cword} -eq 2 ]]; then
        if [[ "${cur}" == -* ]]; then
          compgen -V COMPREPLY -W "-h --help" -- "${cur}"
        else
          compgen -V COMPREPLY -W "ls create info edit close reopen pin unpin delete" -- "${cur}"
        fi
      elif [[ "${words[2]}" == "create" || "${words[2]}" == "edit" ]]; then
        compgen -V COMPREPLY -W "-e --emoji --icon --icon-color -h --help" -- "${cur}"
      else
        compgen -V COMPREPLY -W "-h --help" -- "${cur}"
      fi
      ;;
    file)
      if [[ ${cword} -eq 2 ]]; then
        compgen -V COMPREPLY -W "get" -- "${cur}"
      elif [[ "${words[2]}" == "get" ]]; then
        if [[ "${prev}" == "--type" ]]; then
          compgen -V COMPREPLY -W "photo video doc audio all" -- "${cur}"
        elif [[ "${prev}" == "-o" || "${prev}" == "--output" ]]; then
          compopt -o filenames
          compgen -V COMPREPLY -f -- "${cur}"
        else
          compgen -V COMPREPLY -W "-a --all -A -o --output -t --topic -n --limit --type -h --help" -- "${cur}"
        fi
      fi
      ;;
    completion)
      compgen -V COMPREPLY -W "bash zsh fish" -- "${cur}"
      ;;
    *)
      ;;
  esac

  return 0
}

complete -F _grm_completions grm
