#!/bin/bash

set -e
set -u

uppers="ABCDEFGHIJKLMNOPQRSTUVWXYZ"
lowers="abcdefghijklmnopqrstuvwxyz"
wordBoundaries="._-"

#usage: camelcasify "some_stringHere" -> "some-string-here"
#usage: camelcasify "domHTML" -> "dom-html"
#usage: camelcasify "domHtML" -> "dom-ht-ml"
#usage: camelcasify "dom-HTML" -> "dom-html"
lowerHyphenate(){
  OUTPUT=""
  i=0
  prevWasUpper="true" # Causes leading uppercase
  while ([ $i -lt ${#1} ]) do
    CUR=${1:$i:1}
    case $uppers in
        *$CUR*)
          CUR=${uppers%$CUR*};
          if [[ "${prevWasUpper}" == "false" ]]; then
            OUTPUT="${OUTPUT}-${lowers:${#CUR}:1}"
          else
            # No hyphen
            OUTPUT="${OUTPUT}${lowers:${#CUR}:1}"
          fi
          prevWasUpper="true" # Causes leading uppercase
          ;;
        *)
          OUTPUT="${OUTPUT}$CUR"
          prevWasUpper="false" # Causes leading uppercase
          ;;
    esac
    i=$((i+1))
  done
  echo "${OUTPUT}"
}



# https://stackoverflow.com/a/677212
# We want to check if esy has been installed globally, but only when running
# naked on the command line (just `pesy`).
if hash esy 2>/dev/null; then
  true
else
  printf "\\n%sERROR: You haven't installed esy globally. First install esy then try again.%s" "${RED}${BOLD}" "${RESET}"
  printf "\\n"
  printf "\\n    npm install -g esy"
  printf "\\n\\n"
  exit 1
fi

HAS_JSON="false"
if [ -f  "${PWD}/package.json" ]; then
  HAS_JSON="true"
else
  if [ -f  "${PWD}/esy.json" ]; then
    HAS_JSON="true"
  fi
fi
if [[ "$HAS_JSON" == "true" ]]; then
  printf "\\n%sIncorrect Usage:%s" "${RED}${BOLD}" "${RESET}"
  printf "\\n"
  printf "\\npesy should only be run via 'esy pesy', unless you want to create a new project"
  printf "\\nin a fresh directory. This does not appear to be a fresh directory because"
  printf "\\nit contains a package.json or esy.json file.\n\n"
  printf "\\nType esy --help for more assistance.\n\n"
  exit 1
fi


CUR_DIR_NAME=${PWD##*/}
CUR_DIR_NAME_KEBAB=$(lowerHyphenate "${CUR_DIR_NAME}")
printf "\\n%sCreating new package at ${PWD}/package.json.%s\\n" "${GREEN}${BOLD}" "$RESET"
printf "\\n"
printf "Enter new package name (lowercase/hyphens) [default %s]:" "${CUR_DIR_NAME_KEBAB}"
read ANSWER
if [[ "${ANSWER}" == "" ]]; then
  ANSWER_KEBAB="${CUR_DIR_NAME_KEBAB}"
else
  if [[ "$ANSWER" != "${ANSWER_KEBAB}" ]]; then
    printf "\\n%sPackage names should only consist of lower case and hyphens. Pesy is going to adjust the name to%s:%s" "${YELLOW}${BOLD}" "${RESET}" "${BOLD}${ANSWER_KEBAB}${RESET}"
  fi
  ANSWER_KEBAB=$(lowerHyphenate "${ANSWER}")
fi

# https://stackoverflow.com/a/8952274
source "${PESY_DIR}/pesy-name-utils.sh"

# Gnu uppercasing extensions to sed don't exist on mac.
PACKAGE_NAME_UPPER_CAMEL=$(upperCamelCasify "${ANSWER_KEBAB}")
NAMESPACE="${PACKAGE_NAME_UPPER_CAMEL}"
PUBLIC_LIB_NAME="${ANSWER_KEBAB}.lib"
printf "{\\n"                                                           >> "${PWD}/package.json"
printf "  \"name\": \"%s\",\\n" "$ANSWER_KEBAB"                         >> "${PWD}/package.json"
printf "  \"version\": \"0.0.0\",\\n"                                   >> "${PWD}/package.json"
printf "  \"description\": \"My Project\",\\n"                          >> "${PWD}/package.json"
printf "  \"esy\": {\\n"                                                >> "${PWD}/package.json"
printf "    \"build\": \"pesy\"\\n" >> "${PWD}/package.json"
printf "  },\\n"                                                        >> "${PWD}/package.json"
printf "  \"buildDirs\": {\\n"                                          >> "${PWD}/package.json"
printf "    \"lib\": {\\n"                                              >> "${PWD}/package.json"
printf "      \"name\": \"%s\",\\n" "${PUBLIC_LIB_NAME}"                >> "${PWD}/package.json"
printf "      \"namespace\": \"%s\"\\n" "${PACKAGE_NAME_UPPER_CAMEL}"      >> "${PWD}/package.json"
printf "    },\\n"                                                      >> "${PWD}/package.json"
printf "    \"bin\": {\\n"                                              >> "${PWD}/package.json"
printf "      \"require\": [\"%s\"],\\n" "${PUBLIC_LIB_NAME}"           >> "${PWD}/package.json"
printf "      \"main\": \"%s\",\\n" "Test${PACKAGE_NAME_UPPER_CAMEL}"            >> "${PWD}/package.json"
printf "      \"name\": \"%s\"\\n" "${PACKAGE_NAME_UPPER_CAMEL}.exe"       >> "${PWD}/package.json"
printf "    }\\n"                                                       >> "${PWD}/package.json"
printf "  },\\n"                                                        >> "${PWD}/package.json"
printf "  \"dependencies\": {\\n"                                       >> "${PWD}/package.json"
printf "    \"@opam/dune\": \"*\",\\n"                                  >> "${PWD}/package.json"
printf "    \"@esy-ocaml/reason\": \"*\",\\n"                           >> "${PWD}/package.json"
printf "    \"refmterr\": \"*\",\\n"                                    >> "${PWD}/package.json"
printf "    \"ocaml\": \"~4.6.0\",\\n"                                  >> "${PWD}/package.json"
printf "    \"pesy\": \"*\"\\n"                                         >> "${PWD}/package.json"
printf "  },\\n"                                                        >> "${PWD}/package.json"
printf "  \"devDependencies\": {\\n"                                    >> "${PWD}/package.json"
printf "    \"@esy-ocaml/merlin\": \"*\"\\n"                            >> "${PWD}/package.json"
printf "  }\\n"                                                         >> "${PWD}/package.json"
printf "}\\n"                                                           >> "${PWD}/package.json"

mkdir -p "${PWD}/bin/"
printf "%s;\\n" "${PACKAGE_NAME_UPPER_CAMEL}.Util.foo()"                   >> "${PWD}/bin/Test${PACKAGE_NAME_UPPER_CAMEL}.re"

mkdir -p "${PWD}/lib/"
printf "let foo = () => print_endline(\"Hello\");\\n"                   >> "${PWD}/lib/Util.re"


printf "\\n%sSuccess!%s Created %s\\n\\n" "${BOLD}${GREEN}" "${RESET}" "${PWD}/package.json"
printf "Now, run:\\n"
printf "\\n"
printf "    esy install     # Install Dependencies\\n"
printf "    esy pesy        # Configures Build\\n"
printf "    esy build       # Build Project\\n"
printf "\\n"
exit 0
