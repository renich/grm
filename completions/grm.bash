#!/usr/bin/bash
# Bash completion script for grm (Group & Telegram Manager CLI)
# Generated dynamically by introspection of CommandRegistry

_grm_completions() {
  local cur prev words cword
  _init_completion || return

  local global_opts="-h --help --help=all -V --version -v --verbose -d --debug -q --quiet -c --config -T --test-dc -F --format --color --no-color"
  local commands="login logout chat msg topic file folder search completion"

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
      if [[ ${cword} -ge 2 ]]; then
        COMPREPLY=($(compgen -W "-h --help -p --phone -k --code -q --qr" -- "${cur}"))
      fi
      ;;
    logout)
      if [[ ${cword} -ge 2 ]]; then
        COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
      fi
      ;;
    chat)
      if [[ ${cword} -eq 2 ]]; then
        if [[ "${cur}" == -* ]]; then
          COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
        else
          COMPREPLY=($(compgen -W "ls create info set-title set-desc pin unpin delete" -- "${cur}"))
        fi
      elif [[ "${words[2]}" == "ls" ]]; then
        COMPREPLY=($(compgen -W "-h --help -n --limit -S --since -f --filter -F --folder" -- "${cur}"))
      elif [[ "${words[2]}" == "create" ]]; then
        COMPREPLY=($(compgen -W "-h --help --private --public" -- "${cur}"))
      elif [[ "${words[2]}" == "info" ]]; then
        COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "set-title" ]]; then
        COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "set-desc" ]]; then
        COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "pin" ]]; then
        COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "unpin" ]]; then
        COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "delete" ]]; then
        COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
      fi
      ;;
    msg)
      if [[ ${cword} -eq 2 ]]; then
        if [[ "${cur}" == -* ]]; then
          COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
        else
          COMPREPLY=($(compgen -W "ls send info edit search export pin unpin delete" -- "${cur}"))
        fi
      elif [[ "${words[2]}" == "ls" ]]; then
        COMPREPLY=($(compgen -W "-h --help -t --topic -n --limit -S --since -f --filter -r --reverse" -- "${cur}"))
      elif [[ "${words[2]}" == "send" ]]; then
        COMPREPLY=($(compgen -W "-h --help -a --attach -m --media -C --caption -t --topic -r --reply-to" -- "${cur}"))
      elif [[ "${words[2]}" == "info" ]]; then
        COMPREPLY=($(compgen -W "-h --help -t --topic" -- "${cur}"))
      elif [[ "${words[2]}" == "edit" ]]; then
        COMPREPLY=($(compgen -W "-h --help -t --topic" -- "${cur}"))
      elif [[ "${words[2]}" == "search" ]]; then
        COMPREPLY=($(compgen -W "-h --help -q --query -t --topic -n --limit" -- "${cur}"))
      elif [[ "${words[2]}" == "export" ]]; then
        COMPREPLY=($(compgen -W "-h --help -f --format -o --output -t --topic -n --limit" -- "${cur}"))
      elif [[ "${words[2]}" == "pin" ]]; then
        COMPREPLY=($(compgen -W "-h --help -t --topic" -- "${cur}"))
      elif [[ "${words[2]}" == "unpin" ]]; then
        COMPREPLY=($(compgen -W "-h --help -a --all" -- "${cur}"))
      elif [[ "${words[2]}" == "delete" ]]; then
        COMPREPLY=($(compgen -W "-h --help -e --for-everyone" -- "${cur}"))
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
        COMPREPLY=($(compgen -W "-h --help -n --limit" -- "${cur}"))
      elif [[ "${words[2]}" == "create" ]]; then
        COMPREPLY=($(compgen -W "-h --help -e --emoji --icon-color" -- "${cur}"))
      elif [[ "${words[2]}" == "info" ]]; then
        COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "edit" ]]; then
        COMPREPLY=($(compgen -W "-h --help -e --emoji" -- "${cur}"))
      elif [[ "${words[2]}" == "close" ]]; then
        COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "reopen" ]]; then
        COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "pin" ]]; then
        COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "unpin" ]]; then
        COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "delete" ]]; then
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
        COMPREPLY=($(compgen -W "-h --help -a --all -A -o --output -t --topic -n --limit --type" -- "${cur}"))
      fi
      ;;
    folder)
      if [[ ${cword} -eq 2 ]]; then
        if [[ "${cur}" == -* ]]; then
          COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
        else
          COMPREPLY=($(compgen -W "ls create edit delete" -- "${cur}"))
        fi
      elif [[ "${words[2]}" == "ls" ]]; then
        COMPREPLY=($(compgen -W "-h --help -v --verbose" -- "${cur}"))
      elif [[ "${words[2]}" == "create" ]]; then
        COMPREPLY=($(compgen -W "-h --help -g --include-groups -c --include-channels -b --include-bots -C --include-contacts -N --include-non-contacts --exclude-muted --exclude-read --exclude-archived -i --include-chats -p --pinned-chats" -- "${cur}"))
      elif [[ "${words[2]}" == "edit" ]]; then
        COMPREPLY=($(compgen -W "-h --help -t --title -a --add-chat -r --remove-chat -P --pin-chat -U --unpin-chat" -- "${cur}"))
      elif [[ "${words[2]}" == "delete" ]]; then
        COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
      fi
      ;;
    search)
      if [[ ${cword} -eq 2 ]]; then
        if [[ "${cur}" == -* ]]; then
          COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
        else
          COMPREPLY=($(compgen -W "chats supergroups channels msgs users files" -- "${cur}"))
        fi
      elif [[ "${words[2]}" == "chats" ]]; then
        COMPREPLY=($(compgen -W "-h --help -n --limit -o --offset -v --verbose" -- "${cur}"))
      elif [[ "${words[2]}" == "supergroups" ]]; then
        COMPREPLY=($(compgen -W "-h --help -n --limit -o --offset -v --verbose" -- "${cur}"))
      elif [[ "${words[2]}" == "channels" ]]; then
        COMPREPLY=($(compgen -W "-h --help -n --limit -o --offset -v --verbose" -- "${cur}"))
      elif [[ "${words[2]}" == "msgs" ]]; then
        COMPREPLY=($(compgen -W "-h --help -c --chat -n --limit -o --offset -v --verbose" -- "${cur}"))
      elif [[ "${words[2]}" == "users" ]]; then
        COMPREPLY=($(compgen -W "-h --help -n --limit -o --offset -v --verbose" -- "${cur}"))
      elif [[ "${words[2]}" == "files" ]]; then
        COMPREPLY=($(compgen -W "-h --help -t --type -n --limit -o --offset -v --verbose" -- "${cur}"))
      fi
      ;;
    completion)
      if [[ ${cword} -eq 2 ]]; then
        if [[ "${cur}" == -* ]]; then
          COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
        else
          COMPREPLY=($(compgen -W "bash zsh fish" -- "${cur}"))
        fi
      elif [[ "${words[2]}" == "bash" ]]; then
        COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "zsh" ]]; then
        COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "fish" ]]; then
        COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
      fi
      ;;
    *)
      ;;
  esac

  return 0
}

complete -F _grm_completions grm
