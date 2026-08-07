#!/usr/bin/bash
# Bash completion script for grm (Group & Telegram Manager CLI)
# Generated dynamically by 'grm completion bash'

_grm_completions() {
  local cur prev words cword
  _init_completion || return

  local global_opts="-h --help --help=all -V --version -v --verbose -d --debug -q --quiet -c --config -T --test-dc -F --format --color --no-color"
  local commands="login chat folder search msg topic file completion"

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
        if [[ "${cur}" == -* ]]; then
          COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
        else
          COMPREPLY=($(compgen -W "ls create info set-title set-desc pin unpin delete" -- "${cur}"))
        fi
      elif [[ "${words[2]}" == "ls" ]]; then
        COMPREPLY=($(compgen -W "-n --limit -S --since -f --filter --folder -h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "create" ]]; then
        if [[ ${cword} -eq 3 ]]; then
          COMPREPLY=($(compgen -W "group channel" -- "${cur}"))
        else
          COMPREPLY=($(compgen -W "--private --public -h --help" -- "${cur}"))
        fi
      elif [[ "${words[2]}" == "info" || "${words[2]}" == "delete" || "${words[2]}" == "pin" || "${words[2]}" == "unpin" || "${words[2]}" == "set-title" || "${words[2]}" == "set-desc" ]]; then
        COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
      fi
      ;;
    folder)
      if [[ ${cword} -eq 2 ]]; then
        if [[ "${cur}" == -* ]]; then
          COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
        else
          COMPREPLY=($(compgen -W "ls create edit delete" -- "${cur}"))
        fi
      elif [[ "${words[2]}" == "ls" || "${words[2]}" == "create" || "${words[2]}" == "edit" || "${words[2]}" == "delete" ]]; then
        COMPREPLY=($(compgen -W "-n --limit -h --help" -- "${cur}"))
      fi
      ;;
    search)
      if [[ ${cword} -eq 2 ]]; then
        if [[ "${cur}" == -* ]]; then
          COMPREPLY=($(compgen -W "-n --limit -v --verbose -h --help" -- "${cur}"))
        else
          COMPREPLY=($(compgen -W "chats supergroups msgs users files" -- "${cur}"))
        fi
      elif [[ "${words[2]}" == "chats" || "${words[2]}" == "supergroups" || "${words[2]}" == "users" || "${words[2]}" == "files" ]]; then
        COMPREPLY=($(compgen -W "-n --limit -v --verbose -h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "msgs" ]]; then
        COMPREPLY=($(compgen -W "-c --chat -n --limit -v --verbose -h --help" -- "${cur}"))
      fi
      ;;
    msg)
      if [[ ${cword} -eq 2 ]]; then
        if [[ "${cur}" == -* ]]; then
          COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
        else
          COMPREPLY=($(compgen -W "ls export search send info edit pin unpin delete" -- "${cur}"))
        fi
      elif [[ "${words[2]}" == "ls" ]]; then
        COMPREPLY=($(compgen -W "-n --limit -t --topic -S --since -f --filter --sender -r --reverse -h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "export" ]]; then
        if [[ "${prev}" == "-f" || "${prev}" == "--format" ]]; then
          COMPREPLY=($(compgen -W "csv json" -- "${cur}"))
        elif [[ "${prev}" == "-o" || "${prev}" == "--output" ]]; then
          _filedir
        else
          COMPREPLY=($(compgen -W "-f --format -o --output -t --topic -n --limit -h --help" -- "${cur}"))
        fi
      elif [[ "${words[2]}" == "search" ]]; then
        COMPREPLY=($(compgen -W "-q --query -n --limit -t --topic -S --since -f --filter --sender -h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "send" ]]; then
        if [[ "${prev}" == "-a" || "${prev}" == "--attach" ]]; then
          _filedir
        else
          COMPREPLY=($(compgen -W "-a --attach -m --media -C --caption -t --topic -h --help" -- "${cur}"))
        fi
      elif [[ "${words[2]}" == "edit" || "${words[2]}" == "info" || "${words[2]}" == "pin" ]]; then
        COMPREPLY=($(compgen -W "-t --topic -h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "unpin" ]]; then
        COMPREPLY=($(compgen -W "-a --all -h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "delete" ]]; then
        COMPREPLY=($(compgen -W "--for-everyone -e -h --help" -- "${cur}"))
      fi
      ;;
    topic)
      if [[ ${cword} -eq 2 ]]; then
        if [[ "${cur}" == -* ]]; then
          COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
        else
          COMPREPLY=($(compgen -W "ls create info edit close reopen pin unpin delete" -- "${cur}"))
        fi
      elif [[ "${words[2]}" == "ls" ]]; then
        COMPREPLY=($(compgen -W "-n --limit -S --since -f --filter -h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "create" || "${words[2]}" == "edit" ]]; then
        COMPREPLY=($(compgen -W "-e --emoji --icon --icon-color -h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "info" || "${words[2]}" == "close" || "${words[2]}" == "reopen" || "${words[2]}" == "pin" || "${words[2]}" == "unpin" || "${words[2]}" == "delete" ]]; then
        COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
      fi
      ;;
    file)
      if [[ ${cword} -eq 2 ]]; then
        if [[ "${cur}" == -* ]]; then
          COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
        else
          COMPREPLY=($(compgen -W "get" -- "${cur}"))
        fi
      elif [[ "${words[2]}" == "get" ]]; then
        if [[ "${prev}" == "--type" ]]; then
          COMPREPLY=($(compgen -W "photo video doc audio all" -- "${cur}"))
        elif [[ "${prev}" == "-o" || "${prev}" == "--output" ]]; then
          _filedir
        else
          COMPREPLY=($(compgen -W "-a --all -A -o --output -t --topic -n --limit --type -h --help" -- "${cur}"))
        fi
      fi
      ;;
    completion)
      if [[ "${cur}" == -* ]]; then
        COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
      else
        COMPREPLY=($(compgen -W "bash zsh fish" -- "${cur}"))
      fi
      ;;
    *)
      ;;
  esac

  return 0
}

complete -F _grm_completions grm
