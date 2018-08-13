BIN_DIR="${cur__root}/<DIR>"
BIN_DUNE_FILE="${BIN_DIR}/dune"
# FOR BINARY IN DIRECTORY <DIR>
<DIR>_MAIN_MODULE="${<DIR>_MAIN_MODULE:-$DEFAULT_MAIN_MODULE_NAME}"

<DIR>_MAIN_MODULE_NAME="${<DIR>_MAIN_MODULE%%.*}"
# https://stackoverflow.com/a/965072
if [ "$<DIR>_MAIN_MODULE_NAME"=="$<DIR>_MAIN_MODULE" ]; then
  # If they did not specify an extension, we'll assume it is .re
  <DIR>_MAIN_MODULE_FILENAME="${<DIR>_MAIN_MODULE}.re"
else
  <DIR>_MAIN_MODULE_FILENAME="${<DIR>_MAIN_MODULE}"
fi

if [ -f  "${BIN_DIR}/${<DIR>_MAIN_MODULE_FILENAME}" ]; then
  true
else
  BUILD_STALE_PROBLEM="true"
  notifyUser
  echo ""
  if [ "${MODE}" == "build" ]; then
    printf "    □  Generate %s main module\\n" "${<DIR>_MAIN_MODULE_FILENAME}"
  else
    printf "    %s☒%s  Generate %s main module\\n" "${BOLD}${GREEN}" "${RESET}" "${<DIR>_MAIN_MODULE_FILENAME}"
    mkdir -p "${BIN_DIR}"
    printf "print_endline(\"Hello!\");" > "${BIN_DIR}/${<DIR>_MAIN_MODULE_FILENAME}"
  fi
fi

if [ -d "${BIN_DIR}" ]; then
  LAST_EXE_NAME="<EXE_NAME>"
  BIN_DUNE_EXISTING_CONTENTS=""
  if [ -f "${BIN_DUNE_FILE}" ]; then
    BIN_DUNE_EXISTING_CONTENTS=$(<"${BIN_DUNE_FILE}")
  else
    BIN_DUNE_EXISTING_CONTENTS=""
  fi
  BIN_DUNE_CONTENTS="(executable"
  BIN_DUNE_CONTENTS=$(printf "%s\\n %s" "${BIN_DUNE_CONTENTS}" "  ; !!!! This dune file is generated from the package.json file. Do NOT modify by hand.")
  BIN_DUNE_CONTENTS=$(printf "%s\\n %s" "${BIN_DUNE_CONTENTS}" "  ; !!!! Instead, edit the package.json and then rerun 'esy pesy' at the project root.")
  BIN_DUNE_CONTENTS=$(printf "%s\\n %s" "${BIN_DUNE_CONTENTS}" "  ; The entrypoint module")
  BIN_DUNE_CONTENTS=$(printf "%s\\n %s" "${BIN_DUNE_CONTENTS}" "  (name ${<DIR>_MAIN_MODULE_NAME})  ;  From package.json main field")
  BIN_DUNE_CONTENTS=$(printf "%s\\n %s" "${BIN_DUNE_CONTENTS}" "  (public_name <EXE_NAME>)  ;  From package.json name field")

  if [ -z "${<DIR>_JSOO_FLAGS}" ] && [ -z "${<DIR>_JSOO_FILES}" ]; then
    # No jsoo flags whatsoever
    true
  else
    BIN_DUNE_CONTENTS=$(printf "%s\\n %s" "${BIN_DUNE_CONTENTS}" "  (js_of_ocaml ")
    if [ ! -z "${<DIR>_JSOO_FLAGS}" ]; then
      BIN_DUNE_CONTENTS=$(printf "%s\\n %s\\n" "${BIN_DUNE_CONTENTS}" "    (flags (${<DIR>_JSOO_FLAGS}))  ; From package.json jsooFlags field")
    fi
    if [ ! -z "${<DIR>_JSOO_FILES}" ]; then
      BIN_DUNE_CONTENTS=$(printf "%s\\n %s\\n" "${BIN_DUNE_CONTENTS}" "    (javascript_files ${<DIR>_JSOO_FILES})  ; From package.json jsooFiles field")
    fi
    BIN_DUNE_CONTENTS=$(printf "%s\\n%s" "${BIN_DUNE_CONTENTS}" "   )")
  fi
  if [ ! -z "${<DIR>_REQUIRE}" ]; then
    BIN_DUNE_CONTENTS=$(printf "%s\\n %s\\n" "${BIN_DUNE_CONTENTS}" "  (libraries ${<DIR>_REQUIRE}) ;  From package.json require field (array of strings)")
  fi
  if [ ! -z "${<DIR>_FLAGS}" ]; then
    BIN_DUNE_CONTENTS=$(printf "%s\\n %s\\n" "${BIN_DUNE_CONTENTS}" "  (flags (${<DIR>_FLAGS})) ;  From package.json flags field")
  fi
  if [ ! -z "${<DIR>_OCAMLC_FLAGS}" ]; then
    BIN_DUNE_CONTENTS=$(printf "%s\\n %s\\n" "${BIN_DUNE_CONTENTS}" "  (ocamlc_flags (${<DIR>_OCAMLC_FLAGS}))  ; From package.json ocamlcFlags field")
  fi
  if [ ! -z "${<DIR>_OCAMLOPT_FLAGS}" ]; then
    BIN_DUNE_CONTENTS=$(printf "%s\\n %s\\n" "${BIN_DUNE_CONTENTS}" "  (ocamlopt_flags (${<DIR>_OCAMLOPT_FLAGS}))  ; From package.json ocamloptFlags field")
  fi
  BIN_DUNE_CONTENTS=$(printf "%s\\n%s\\n" "${BIN_DUNE_CONTENTS}" "  )")

  if [ "${BIN_DUNE_EXISTING_CONTENTS}" == "${BIN_DUNE_CONTENTS}" ]; then
    true
  else
    notifyUser
    BUILD_STALE_PROBLEM="true"
    if [ "${MODE}" == "build" ]; then
      printf "    □  Update <DIR>/dune build config\\n"
    else
      printf "    %s☒%s  Update <DIR>/dune build config\\n" "${BOLD}${GREEN}" "${RESET}"
      printf "%s" "${BIN_DUNE_CONTENTS}" > "${BIN_DUNE_FILE}"
      mkdir -p "${BIN_DIR}"
    fi
  fi
else
  BUILD_STALE_PROBLEM="true"
  notifyUser
  if [ "${MODE}" == "build" ]; then
    printf "    □  Generate missing the <DIR> directory described in package.json buildDirs\\n"
  else
    printf "    %s☒%s  Generate missing the <DIR> directory described in package.json buildDirs\\n" "${BOLD}${GREEN}" "${RESET}"
    mkdir -p "${BIN_DIR}"
  fi
fi
