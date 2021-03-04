#!/usr/bin/env bash
cd "$(dirname "$(readlink -f "$1")")" || exit 1
IFS=$'\n' read -r -d '' -a files <<< "$(
	tr '\n' '' < "$1" | sed "s| *\\\ *||g" | tr '' '\n' \
	| grep -E "^HEADERS +\+=|^SOURCES +\+=|^FORMS +\+=" \
	| sed -E "
		s|^HEADERS +\+= *||g;
		s|^SOURCES +\+= *||g;
		s|^FORMS +\+= *||g;
	" | tr '' '\n' | sed '/^\s*$/d'
)"
lupdate "${files[@]}" -ts "$2"
