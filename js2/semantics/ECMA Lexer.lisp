;;; The contents of this file are subject to the Netscape Public License
;;; Version 1.0 (the "NPL"); you may not use this file except in
;;; compliance with the NPL.  You may obtain a copy of the NPL at
;;; http://www.mozilla.org/NPL/
;;;
;;; Software distributed under the NPL is distributed on an "AS IS" basis,
;;; WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
;;; for the specific language governing rights and limitations under the
;;; NPL.
;;;
;;; The Initial Developer of this code under the NPL is Netscape
;;; Communications Corporation.  Portions created by Netscape are
;;; Copyright (C) 1998 Netscape Communications Corporation.  All Rights
;;; Reserved.

;;;
;;; ECMAScript sample lexer
;;;
;;; Waldemar Horwat (waldemar@netscape.com)
;;;

(defun digit-char-16 (char)
  (assert-non-null (digit-char-p char 16)))


(progn
  (defparameter *lw*
    (generate-world
     "L"
     '((lexer code-lexer
              :lalr-1
              :next-token
              ((:white-space-character (#?0009 #?000B #?000C #\space) ())
               (:line-terminator (#?000A #?000D) ())
               (:non-terminator (- every :line-terminator) ())
               (:non-terminator-or-slash (- :non-terminator (#\/)) ())
               (:non-terminator-or-asterisk-or-slash (- :non-terminator (#\* #\/)) ())
               (:identifier-letter (++ (#\A #\B #\C #\D #\E #\F #\G #\H #\I #\J #\K #\L #\M #\N #\O #\P #\Q #\R #\S #\T #\U #\V #\W #\X #\Y #\Z)
                                       (#\a #\b #\c #\d #\e #\f #\g #\h #\i #\j #\k #\l #\m #\n #\o #\p #\q #\r #\s #\t #\u #\v #\w #\x #\y #\z)
                                       (#\$ #\_))
                                   ((character-value character-value)))
               (:decimal-digit (#\0 #\1 #\2 #\3 #\4 #\5 #\6 #\7 #\8 #\9)
                               ((character-value character-value)
                                (decimal-value digit-value)))
               (:non-zero-digit (#\1 #\2 #\3 #\4 #\5 #\6 #\7 #\8 #\9)
                                ((decimal-value digit-value)))
               (:octal-digit (#\0 #\1 #\2 #\3 #\4 #\5 #\6 #\7)
                             ((character-value character-value)
                              (octal-value digit-value)))
               (:zero-to-three (#\0 #\1 #\2 #\3)
                               ((octal-value digit-value)))
               (:four-to-seven (#\4 #\5 #\6 #\7)
                               ((octal-value digit-value)))
               (:hex-digit (#\0 #\1 #\2 #\3 #\4 #\5 #\6 #\7 #\8 #\9 #\A #\B #\C #\D #\E #\F #\a #\b #\c #\d #\e #\f)
                           ((hex-value digit-value)))
               (:exponent-indicator (#\E #\e) ())
               (:hex-indicator (#\X #\x) ())
               (:plain-string-char (- every (+ (#\' #\" #\\) :octal-digit :line-terminator))
                                   ((character-value character-value)))
               (:string-non-escape (- :non-terminator (+ :octal-digit (#\x #\u #\' #\" #\\ #\b #\f #\n #\r #\t #\v)))
                                   ((character-value character-value))))
              ((character-value character identity (*))
               (digit-value integer digit-char-16 ((:global-variable "digitValue") "(" * ")"))))
       
       (%section "Comments")
       (production :line-comment (#\/ #\/ :line-comment-characters) line-comment)
       
       (production :line-comment-characters () line-comment-characters-empty)
       (production :line-comment-characters (:line-comment-characters :non-terminator) line-comment-characters-chars)
       (%charclass :non-terminator)
       
       (production :single-line-block-comment (#\/ #\* :block-comment-characters #\* #\/) single-line-block-comment)
       
       (production :block-comment-characters () block-comment-characters-empty)
       (production :block-comment-characters (:block-comment-characters :non-terminator-or-slash) block-comment-characters-chars)
       (production :block-comment-characters (:pre-slash-characters #\/) block-comment-characters-slash)
       
       (production :pre-slash-characters () pre-slash-characters-empty)
       (production :pre-slash-characters (:block-comment-characters :non-terminator-or-asterisk-or-slash) pre-slash-characters-chars)
       (production :pre-slash-characters (:pre-slash-characters #\/) pre-slash-characters-slash)
       
       (%charclass :non-terminator-or-slash)
       (%charclass :non-terminator-or-asterisk-or-slash)
       
       (production :multi-line-block-comment (#\/ #\* :multi-line-block-comment-characters :block-comment-characters #\* #\/) multi-line-block-comment)
       
       (production :multi-line-block-comment-characters (:block-comment-characters :line-terminator) multi-line-block-comment-characters-first)
       (production :multi-line-block-comment-characters (:multi-line-block-comment-characters :block-comment-characters :line-terminator)
                   multi-line-block-comment-characters-rest)
       
       (%section "White space")
       
       (production :white-space () white-space-empty)
       (production :white-space (:white-space :white-space-character) white-space-character)
       (production :white-space (:white-space :single-line-block-comment) white-space-single-line-block-comment)
       (%charclass :white-space-character)
       
       (%section "Line breaks")
       
       (production :line-break (:line-terminator) line-break-line-terminator)
       (production :line-break (:line-comment :line-terminator) line-break-line-comment)
       (production :line-break (:multi-line-block-comment) line-break-multi-line-block-comment)
       (%charclass :line-terminator)
       
       (production :line-breaks (:line-break) line-breaks-first)
       (production :line-breaks (:line-breaks :white-space :line-break) line-breaks-rest)
       
       (%section "Tokens")
       
       (declare-action token :next-token token)
       (production :next-token (:white-space :token) next-token
         (token (token :token)))
       
       (declare-action token :token token)
       (production :token (:line-breaks) token-line-breaks
         (token (oneof line-breaks)))
       (production :token (:identifier-or-reserved-word) token-identifier-or-reserved-word
         (token (token :identifier-or-reserved-word)))
       (production :token (:punctuator) token-punctuator
         (token (oneof punctuator (punctuator :punctuator))))
       (production :token (:numeric-literal) token-numeric-literal
         (token (oneof number (double-value :numeric-literal))))
       (production :token (:string-literal) token-string-literal
         (token (oneof string (string-value :string-literal))))
       (production :token (:end-of-input) token-end
         (token (oneof end)))
       
       (production :end-of-input ($end) end-of-input-end)
       (production :end-of-input (:line-comment $end) end-of-input-line-comment)
       
       (deftype token (oneof (identifier string) (reserved-word string) (punctuator string) (number double) (string string) line-breaks end))
       (%print-actions)
       
       (%section "Keywords")
       
       (declare-action name :identifier-name string)
       (production :identifier-name (:identifier-letter) identifier-name-letter
         (name (vector (character-value :identifier-letter))))
       (production :identifier-name (:identifier-name :identifier-letter) identifier-name-next-letter
         (name (append (name :identifier-name) (vector (character-value :identifier-letter)))))
       (production :identifier-name (:identifier-name :decimal-digit) identifier-name-next-digit
         (name (append (name :identifier-name) (vector (character-value :decimal-digit)))))
       (%charclass :identifier-letter)
       (%charclass :decimal-digit)
       (%print-actions)
       
       (define keywords (vector string)
         (vector "break" "case" "catch" "continue" "default" "delete" "do" "else" "finally" "for" "function" "if" "in"
                 "new" "return" "switch" "this" "throw" "try" "typeof" "var" "void" "while" "with"))
       (define future-reserved-words (vector string)
         (vector "class" "const" "debugger" "enum" "export" "extends" "import" "super"))
       (define literals (vector string)
         (vector "null" "true" "false"))
       (define reserved-words (vector string)
         (append keywords (append future-reserved-words literals)))
       
       (define (member (id string) (list (vector string))) boolean
         (if (empty list)
           false
           (let ((s string (first list)))
             (if (string-equal id s)
               true
               (member id (rest list))))))
       
       (declare-action token :identifier-or-reserved-word token)
       (production :identifier-or-reserved-word (:identifier-name) identifier-or-reserved-word-identifier-name
         (token (let ((id string (name :identifier-name)))
                  (if (member id reserved-words)
                    (oneof reserved-word id)
                    (oneof identifier id)))))
       (%print-actions)
       
       (%section "Punctuators")
       
       (declare-action punctuator :punctuator string)
       (production :punctuator (#\=) punctuator-assignment (punctuator "="))
       (production :punctuator (#\>) punctuator-greater-than (punctuator ">"))
       (production :punctuator (#\<) punctuator-less-than (punctuator "<"))
       (production :punctuator (#\= #\=) punctuator-equal (punctuator "=="))
       (production :punctuator (#\= #\= #\=) punctuator-identical (punctuator "==="))
       (production :punctuator (#\< #\=) punctuator-less-than-or-equal (punctuator "<="))
       (production :punctuator (#\> #\=) punctuator-greater-than-or-equal (punctuator ">="))
       (production :punctuator (#\! #\=) punctuator-not-equal (punctuator "!="))
       (production :punctuator (#\! #\= #\=) punctuator-not-identical (punctuator "!=="))
       (production :punctuator (#\,) punctuator-comma (punctuator ","))
       (production :punctuator (#\!) punctuator-not (punctuator "!"))
       (production :punctuator (#\~) punctuator-complement (punctuator "~"))
       (production :punctuator (#\?) punctuator-question (punctuator "?"))
       (production :punctuator (#\:) punctuator-colon (punctuator ":"))
       (production :punctuator (#\.) punctuator-period (punctuator "."))
       (production :punctuator (#\& #\&) punctuator-logical-and (punctuator "&&"))
       (production :punctuator (#\| #\|) punctuator-logical-or (punctuator "||"))
       (production :punctuator (#\+ #\+) punctuator-increment (punctuator "++"))
       (production :punctuator (#\- #\-) punctuator-decrement (punctuator "--"))
       (production :punctuator (#\+) punctuator-plus (punctuator "+"))
       (production :punctuator (#\-) punctuator-minus (punctuator "-"))
       (production :punctuator (#\*) punctuator-times (punctuator "*"))
       (production :punctuator (#\/) punctuator-divide (punctuator "/"))
       (production :punctuator (#\&) punctuator-and (punctuator "&"))
       (production :punctuator (#\|) punctuator-or (punctuator "|"))
       (production :punctuator (#\^) punctuator-xor (punctuator "^"))
       (production :punctuator (#\%) punctuator-modulo (punctuator "%"))
       (production :punctuator (#\< #\<) punctuator-left-shift (punctuator "<<"))
       (production :punctuator (#\> #\>) punctuator-right-shift (punctuator ">>"))
       (production :punctuator (#\> #\> #\>) punctuator-logical-right-shift (punctuator ">>>"))
       (production :punctuator (#\+ #\=) punctuator-plus-equals (punctuator "+="))
       (production :punctuator (#\- #\=) punctuator-minus-equals (punctuator "-="))
       (production :punctuator (#\* #\=) punctuator-times-equals (punctuator "*="))
       (production :punctuator (#\/ #\=) punctuator-divide-equals (punctuator "/="))
       (production :punctuator (#\& #\=) punctuator-and-equals (punctuator "&="))
       (production :punctuator (#\| #\=) punctuator-or-equals (punctuator "|="))
       (production :punctuator (#\^ #\=) punctuator-xor-equals (punctuator "^="))
       (production :punctuator (#\% #\=) punctuator-modulo-equals (punctuator "%="))
       (production :punctuator (#\< #\< #\=) punctuator-left-shift-equals (punctuator "<<="))
       (production :punctuator (#\> #\> #\=) punctuator-right-shift-equals (punctuator ">>="))
       (production :punctuator (#\> #\> #\> #\=) punctuator-logical-right-shift-equals (punctuator ">>>="))
       (production :punctuator (#\() punctuator-open-parenthesis (punctuator "("))
       (production :punctuator (#\)) punctuator-close-parenthesis (punctuator ")"))
       (production :punctuator (#\{) punctuator-open-brace (punctuator "{"))
       (production :punctuator (#\}) punctuator-close-brace (punctuator "}"))
       (production :punctuator (#\[) punctuator-open-bracket (punctuator "["))
       (production :punctuator (#\]) punctuator-close-bracket (punctuator "]"))
       (production :punctuator (#\;) punctuator-semicolon (punctuator ";"))
       (%print-actions)
       
       (%section "Numeric literals")
       
       (declare-action double-value :numeric-literal double)
       (production :numeric-literal (:decimal-literal) numeric-literal-decimal
         (double-value (rational-to-double (rational-value :decimal-literal))))
       (production :numeric-literal (:hex-integer-literal) numeric-literal-hex
         (double-value (rational-to-double (integer-to-rational (integer-value :hex-integer-literal)))))
       (production :numeric-literal (:octal-integer-literal) numeric-literal-octal
         (double-value (rational-to-double (integer-to-rational (integer-value :octal-integer-literal)))))
       (%print-actions)
       
       (define (expt (base rational) (exponent integer)) rational
         (if (= exponent 0)
           (integer-to-rational 1)
           (if (< exponent 0)
             (rational/ (integer-to-rational 1) (expt base (neg exponent)))
             (rational* base (expt base (- exponent 1))))))
       
       (declare-action rational-value :decimal-literal rational)
       (production :decimal-literal (:mantissa :exponent) decimal-literal
         (rational-value (rational* (rational-value :mantissa) (expt (integer-to-rational 10) (integer-value :exponent)))))
       
       (declare-action rational-value :mantissa rational)
       (production :mantissa (:decimal-integer-literal) mantissa-integer
         (rational-value (integer-to-rational (integer-value :decimal-integer-literal))))
       (production :mantissa (:decimal-integer-literal #\.) mantissa-integer-dot
         (rational-value (integer-to-rational (integer-value :decimal-integer-literal))))
       (production :mantissa (:decimal-integer-literal #\. :fraction) mantissa-integer-dot-fraction
         (rational-value (rational+ (integer-to-rational (integer-value :decimal-integer-literal))
                                    (rational-value :fraction))))
       (production :mantissa (#\. :fraction) mantissa-dot-fraction
         (rational-value (rational-value :fraction)))
       
       (declare-action integer-value :decimal-integer-literal integer)
       (production :decimal-integer-literal (#\0) decimal-integer-literal-0
         (integer-value 0))
       (production :decimal-integer-literal (:non-zero-decimal-digits) decimal-integer-literal-nonzero
         (integer-value (integer-value :non-zero-decimal-digits)))
       
       (declare-action integer-value :non-zero-decimal-digits integer)
       (production :non-zero-decimal-digits (:non-zero-digit) non-zero-decimal-digits-first
         (integer-value (decimal-value :non-zero-digit)))
       (production :non-zero-decimal-digits (:non-zero-decimal-digits :decimal-digit) non-zero-decimal-digits-rest
         (integer-value (+ (* 10 (integer-value :non-zero-decimal-digits)) (decimal-value :decimal-digit))))
       
       (%charclass :non-zero-digit)
       
       (declare-action rational-value :fraction rational)
       (production :fraction (:decimal-digits) fraction-decimal-digits
         (rational-value (rational/ (integer-to-rational (integer-value :decimal-digits))
                                    (expt (integer-to-rational 10) (n-digits :decimal-digits)))))
       (%print-actions)
       
       (declare-action integer-value :exponent integer)
       (production :exponent () exponent-none
         (integer-value 0))
       (production :exponent (:exponent-indicator :signed-integer) exponent-integer
         (integer-value (integer-value :signed-integer)))
       (%charclass :exponent-indicator)
       
       (declare-action integer-value :signed-integer integer)
       (production :signed-integer (:decimal-digits) signed-integer-no-sign
         (integer-value (integer-value :decimal-digits)))
       (production :signed-integer (#\+ :decimal-digits) signed-integer-plus
         (integer-value (integer-value :decimal-digits)))
       (production :signed-integer (#\- :decimal-digits) signed-integer-minus
         (integer-value (neg (integer-value :decimal-digits))))
       (%print-actions)
       
       (declare-action integer-value :decimal-digits integer)
       (declare-action n-digits :decimal-digits integer)
       (production :decimal-digits (:decimal-digit) decimal-digits-first
         (integer-value (decimal-value :decimal-digit))
         (n-digits 1))
       (production :decimal-digits (:decimal-digits :decimal-digit) decimal-digits-rest
         (integer-value (+ (* 10 (integer-value :decimal-digits)) (decimal-value :decimal-digit)))
         (n-digits (+ (n-digits :decimal-digits) 1)))
       (%print-actions)
       
       (declare-action integer-value :hex-integer-literal integer)
       (production :hex-integer-literal (#\0 :hex-indicator :hex-digit) hex-integer-literal-first
         (integer-value (hex-value :hex-digit)))
       (production :hex-integer-literal (:hex-integer-literal :hex-digit) hex-integer-literal-rest
         (integer-value (+ (* 16 (integer-value :hex-integer-literal)) (hex-value :hex-digit))))
       (%charclass :hex-indicator)
       (%charclass :hex-digit)
       
       (declare-action integer-value :octal-integer-literal integer)
       (production :octal-integer-literal (#\0 :octal-digit) octal-integer-literal-first
         (integer-value (octal-value :octal-digit)))
       (production :octal-integer-literal (:octal-integer-literal :octal-digit) octal-integer-literal-rest
         (integer-value (+ (* 8 (integer-value :octal-integer-literal)) (octal-value :octal-digit))))
       (%charclass :octal-digit)
       (%print-actions)
       
       (%section "String literals")
       
       (grammar-argument :quote single double)
       (declare-action string-value :string-literal string)
       (production :string-literal (#\' (:string-chars single) #\') string-literal-single
         (string-value (string-value :string-chars)))
       (production :string-literal (#\" (:string-chars double) #\") string-literal-double
         (string-value (string-value :string-chars)))
       (%print-actions)
       
       (declare-action string-value (:string-chars :quote) string)
       (production (:string-chars :quote) ((:ordinary-string-chars :quote)) string-chars-ordinary
         (string-value (string-value :ordinary-string-chars)))
       (production (:string-chars :quote) ((:string-chars :quote) #\\ :short-octal-escape) string-chars-short-escape
         (string-value (append (string-value :string-chars)
                               (vector (character-value :short-octal-escape)))))
       
       (declare-action string-value (:ordinary-string-chars :quote) string)
       (production (:ordinary-string-chars :quote) () ordinary-string-chars-empty
         (string-value ""))
       (production (:ordinary-string-chars :quote) ((:string-chars :quote) :plain-string-char) ordinary-string-chars-char
         (string-value (append (string-value :string-chars)
                               (vector (character-value :plain-string-char)))))
       (production (:ordinary-string-chars :quote) ((:string-chars :quote) (:plain-string-quote :quote)) ordinary-string-chars-quote
         (string-value (append (string-value :string-chars)
                               (vector (character-value :plain-string-quote)))))
       (production (:ordinary-string-chars :quote) ((:ordinary-string-chars :quote) :octal-digit) ordinary-string-chars-octal
         (string-value (append (string-value :ordinary-string-chars)
                               (vector (character-value :octal-digit)))))
       (production (:ordinary-string-chars :quote) ((:string-chars :quote) #\\ :ordinary-escape) ordinary-string-chars-escape
         (string-value (append (string-value :string-chars)
                               (vector (character-value :ordinary-escape)))))
       
       (%charclass :plain-string-char)
       
       (declare-action character-value (:plain-string-quote :quote) character)
       (production (:plain-string-quote single) (#\") plain-string-quote-single
         (character-value #\"))
       (production (:plain-string-quote double) (#\') plain-string-quote-double
         (character-value #\'))
       (%print-actions)
       
       (declare-action character-value :ordinary-escape character)
       (production :ordinary-escape (:string-char-escape) ordinary-escape-character
         (character-value (character-value :string-char-escape)))
       (production :ordinary-escape (:full-octal-escape) ordinary-escape-full-octal
         (character-value (character-value :full-octal-escape)))
       (production :ordinary-escape (:hex-escape) ordinary-escape-hex
         (character-value (character-value :hex-escape)))
       (production :ordinary-escape (:unicode-escape) ordinary-escape-unicode
         (character-value (character-value :unicode-escape)))
       (production :ordinary-escape (:string-non-escape) ordinary-escape-non-escape
         (character-value (character-value :string-non-escape)))
       (%charclass :string-non-escape)
       (%print-actions)
       
       (declare-action character-value :string-char-escape character)
       (production :string-char-escape (#\') string-char-escape-single-quote (character-value #\'))
       (production :string-char-escape (#\") string-char-escape-double-quote (character-value #\"))
       (production :string-char-escape (#\\) string-char-escape-backslash (character-value #\\))
       (production :string-char-escape (#\b) string-char-escape-backspace (character-value #?0008))
       (production :string-char-escape (#\f) string-char-escape-form-feed (character-value #?000C))
       (production :string-char-escape (#\n) string-char-escape-new-line (character-value #?000A))
       (production :string-char-escape (#\r) string-char-escape-return (character-value #?000D))
       (production :string-char-escape (#\t) string-char-escape-tab (character-value #?0009))
       (production :string-char-escape (#\v) string-char-escape-vertical-tab (character-value #?000B))
       (%print-actions)
       
       (declare-action character-value :short-octal-escape character)
       (production :short-octal-escape (:octal-digit) short-octal-escape-1
         (character-value (code-to-character (octal-value :octal-digit))))
       (production :short-octal-escape (:zero-to-three :octal-digit) short-octal-escape-2
         (character-value (code-to-character (+ (* 8 (octal-value :zero-to-three))
                                                (octal-value :octal-digit)))))
       
       (declare-action character-value :full-octal-escape character)
       (production :full-octal-escape (:four-to-seven :octal-digit) full-octal-escape-2
         (character-value (code-to-character (+ (* 8 (octal-value :four-to-seven))
                                                (octal-value :octal-digit)))))
       (production :full-octal-escape (:zero-to-three :octal-digit :octal-digit) full-octal-escape-3
         (character-value (code-to-character (+ (+ (* 64 (octal-value :zero-to-three))
                                                   (* 8 (octal-value :octal-digit 1)))
                                                (octal-value :octal-digit 2)))))
       (%charclass :zero-to-three)
       (%charclass :four-to-seven)
       
       (declare-action character-value :hex-escape character)
       (production :hex-escape (#\x :hex-digit :hex-digit) hex-escape-2
         (character-value (code-to-character (+ (* 16 (hex-value :hex-digit 1))
                                                (hex-value :hex-digit 2)))))
       
       (declare-action character-value :unicode-escape character)
       (production :unicode-escape (#\u :hex-digit :hex-digit :hex-digit :hex-digit) unicode-escape-4
         (character-value (code-to-character (+ (+ (+ (* 4096 (hex-value :hex-digit 1))
                                                      (* 256 (hex-value :hex-digit 2)))
                                                   (* 16 (hex-value :hex-digit 3)))
                                                (hex-value :hex-digit 4)))))
       (%print-actions)
       
       )))
  
  (defparameter *ll* (world-lexer *lw* 'code-lexer))
  (defparameter *lg* (lexer-grammar *ll*))
  (set-up-lexer-metagrammar *ll*)
  (defparameter *lm* (lexer-metagrammar *ll*)))

#|
(depict-rtf-to-local-file
 "ECMA Lexer Charclasses.rtf"
 #'(lambda (rtf-stream)
     (depict-paragraph (rtf-stream ':grammar-header)
       (depict rtf-stream "Character Classes"))
     (dolist (charclass (lexer-charclasses *ll*))
       (depict-charclass rtf-stream charclass))
     (depict-paragraph (rtf-stream ':grammar-header)
       (depict rtf-stream "Grammar"))
     (depict-grammar rtf-stream *lg*)))

(depict-rtf-to-local-file
 "ECMA Lexer.rtf"
 #'(lambda (rtf-stream)
     (depict-world-commands rtf-stream *lw*)))

(depict-html-to-local-file
 "ECMA Lexer.html"
 #'(lambda (rtf-stream)
     (depict-world-commands rtf-stream *lw*))
 "ECMA Lexer")

(with-local-output (s "ECMA Lexer.txt") (print-lexer *ll* s) (print-grammar *lg* s))

(print-illegal-strings m)

(lexer-pparse *ll* "0x20")
(lexer-pparse *ll* "2b")
(lexer-pparse *ll* " 3.75" :trace t)
(lexer-pparse *ll* "25" :trace :code)
(lexer-pmetaparse *ll* "32+abc//23e-a4*7e-2 3 id4 4ef;")
(lexer-pmetaparse *ll* "32+abc//23e-a4*7e-2 3 id4 4ef;
")
(lexer-pmetaparse *ll* "32+abc/ /23e-a4*7e-2 3 /*id4 4*-/ef;

fjds*/y//z")
(lexer-pmetaparse *ll* "3a+in'a+b\\147\"de'\"'\"")
|#


; Return the ECMAScript input string as a list of tokens like:
;   (($number . 3.0) + - ++ else ($string . "a+bgde") ($end))
; Line breaks are removed.
(defun tokenize (string)
  (delete
   '($line-breaks)
   (mapcar
    #'(lambda (token-value)
        (let ((token-value (car token-value)))
          (ecase (car token-value)
            (identifier (cons '$identifier (cdr token-value)))
            ((reserved-word punctuator) (intern (string-upcase (cdr token-value))))
            (number (cons '$number (cdr token-value)))
            (string (cons '$string (cdr token-value)))
            (line-breaks '($line-breaks))
            (end '($end)))))
    (lexer-metaparse *ll* string))
   :test #'equal))


