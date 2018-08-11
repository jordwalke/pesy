
# Perform validation:

LIB_DIR="${cur__root}/<DIR>"
LIB_DUNE_FILE="${LIB_DIR}/dune"

# TODO: Error if there are multiple libraries all using the default namespace.
if [ -d "${LIB_DIR}" ]; then
  true
else
  BUILD_STALE_PROBLEM="true"
  notifyUser
  if [ "${MODE}" == "build" ]; then
    printf "    □  Your project is missing the <DIR> directory described in package.json buildDirs\\n"
  else
    printf "    %s☒%s  Your project is missing the <DIR> directory described in package.json buildDirs\\n" "${BOLD}${GREEN}" "${RESET}"
    mkdir -p "${LIB_DIR}"
  fi
fi

LIB_DUNE_CONTENTS=""
LIB_DUNE_EXISTING_CONTENTS=""
if [ -f "${LIB_DUNE_FILE}" ]; then
  LIB_DUNE_EXISTING_CONTENTS=$(<"${LIB_DUNE_FILE}")
fi
LIB_DUNE_CONTENTS="(library"
LIB_DUNE_CONTENTS=$(printf "%s\\n %s" "${LIB_DUNE_CONTENTS}" "  ; !!!! This dune file is generated from the package.json file. Do NOT modify by hand.")
LIB_DUNE_CONTENTS=$(printf "%s\\n %s" "${LIB_DUNE_CONTENTS}" "  ; !!!! Instead, edit the package.json and then rerun 'esy pesy' at the project root.")
LIB_DUNE_CONTENTS=$(printf "%s\\n %s" "${LIB_DUNE_CONTENTS}" "  ; The namespace other code see this as")
LIB_DUNE_CONTENTS=$(printf "%s\\n %s" "${LIB_DUNE_CONTENTS}" "  (name ${<DIR>_NAMESPACE})")
LIB_DUNE_CONTENTS=$(printf "%s\\n %s" "${LIB_DUNE_CONTENTS}" "  (public_name <LIB_NAME>)")
LIB_DUNE_CONTENTS=$(printf "%s\\n %s\\n" "${LIB_DUNE_CONTENTS}" "  (libraries ${<DIR>_REQUIRE})")
LIB_DUNE_CONTENTS=$(printf "%s\\n %s\\n" "${LIB_DUNE_CONTENTS}" "  (c_names ${<DIR>_C_NAMES})  ; From package.json cNames field")
LIB_DUNE_CONTENTS=$(printf "%s\\n %s\\n" "${LIB_DUNE_CONTENTS}" "  (flags (:standard ${<DIR>_FLAGS}))  ; From package.json flags field")
LIB_DUNE_CONTENTS=$(printf "%s\\n %s\\n" "${LIB_DUNE_CONTENTS}" "  (ocamlc_flags (:standard ${<DIR>_OCAMLC_FLAGS}))  ; From package.json ocamlcFlags field")
LIB_DUNE_CONTENTS=$(printf "%s\\n %s\\n" "${LIB_DUNE_CONTENTS}" "  (ocamlopt_flags (:standard ${<DIR>_OCAMLOPT_FLAGS}))) ; From package.json ocamloptFlags")
if [ "${LIB_DUNE_EXISTING_CONTENTS}" == "${LIB_DUNE_CONTENTS}" ]; then
  true
else
  notifyUser
  BUILD_STALE_PROBLEM="true"
  if [ "${MODE}" == "build" ]; then
    printf "    □  Update <DIR>/dune build config\\n"
  else
    printf "    %s☒%s  Update <DIR>/dune build config\\n" "${BOLD}${GREEN}" "${RESET}"
    printf "%s" "$LIB_DUNE_CONTENTS" > "${LIB_DUNE_FILE}"
  fi
fi
