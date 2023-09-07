% Generic class for SINTEF documents with LaTeX.
%
% by Federico Zenith, federico.zenith@sintef.no.

\ProvidesClass{sintefdoc}[2021/11/23 A generic class for SINTEF documents]

\RequirePackage{etoolbox}

% Whether the language is English;
% defaults to true
\newbool{english}
\booltrue{english}

% Whether to include a field for ISBN where relevant (only public reports)
% defaults to false
\newbool{useISBN}

% Whether to use fontspec for font selection (XeTeX and LuaTeX)
% defaults to false
\newbool{fontspec}

% Whether the document is meant for printing, i.e. using CMYK.
% defaults to false, i.e. RGB display
\newbool{print}

% Force use of Calibri font, defaults to false
\newbool{calibri}

%%% BEGIN Options section; no \RequirePackage or \LoadClass in here %%%

% Command to provide alternative translations in English and Norwegian
\newcommand{\EnglishNorsk}[2]{\ifbool{english}{#1}{#2}}

% Classification options, internal is default as per SINTEF policy
\newcommand*{\@classification}{\EnglishNorsk{Internal}{Intern}}
\newcommand*{\@parentClass}{article}
\DeclareOption{unrestricted} {%
  \renewcommand*{\@classification}{\EnglishNorsk{Unrestricted}{Åpen}}%
  \booltrue{useISBN}
}
\DeclareOption{internal}{%
  \renewcommand*{\@classification}{\EnglishNorsk{Internal}{Intern}}%
}
\DeclareOption{restricted}{%
  \renewcommand*{\@classification}{\EnglishNorsk{Restricted}{Fortrolig}}%
}
\DeclareOption{confidential}{%
  \renewcommand*{\@classification}{\EnglishNorsk{Confidential}{Strengt fortrolig}}%
}
\DeclareOption{report}{%
  \renewcommand{\@parentClass}{report}
}
\DeclareOption{article}{%
  \renewcommand{\@parentClass}{article}
}
\DeclareOption{english}  {\booltrue{english}}
\DeclareOption{norsk}    {\boolfalse{english}}
\DeclareOption{twocolumn}{\OptionNotUsed}
\DeclareOption{display}  {\boolfalse{print}}
\DeclareOption{print}    {\booltrue{print}\PassOptionsToPackage{cmyk}{xcolor}}
\DeclareOption{calibri}  {\booltrue{calibri}\booltrue{fontspec}}
\DeclareOption*{\PassOptionsToClass{\CurrentOption}{\@parentClass}}
\ProcessOptions\relax

%%% END Options section %%%

\LoadClass[a4paper,11pt]{\@parentClass}

% Use a prettier date format, and adapt according to language.
\RequirePackage[useregional]{datetime2}
% If English is selected, prefer British, but have American ready.
\RequirePackage[\ifbool{english}{main=british,american}{}]{babel}

% Check what compiler is running so we know how to handle fonts later
\RequirePackage{iftex}
\ifXeTeX \booltrue{fontspec}\else\fi
\ifLuaTeX\booltrue{fontspec}\else\fi

% Set fonts.
% Carlito is a free clone of Calibri; "lining" option for "normal" digits.
% STIX 2 is an advanced open font, broadly compatible with Times New Roman.
% it also has great math rendering.
\ifbool{fontspec}{ % XeLaTeX or LuaLaTeX, advanced font handling
  \RequirePackage{fontspec,unicode-math}
  \defaultfontfeatures{Ligatures=TeX}
  \setmainfont{STIX Two Text}
  \setmathfont{STIX Two Math}
  \setsansfont{\ifbool{calibri}{Calibri}{Carlito}}
}{ % Ye goode olde LaTeX and PDFLaTeX
  \RequirePackage[utf8]{inputenc}
  \RequirePackage[T1]{fontenc}
  \RequirePackage{stix2}
  \RequirePackage[lining]{carlito}
}
% Use sans font by default
\renewcommand{\familydefault}{\sfdefault}

% Define \and function for multiple authors
\renewcommand{\and}{\\*}

% Define the main logo, whether CMYK (print) or RGB (screen)
\RequirePackage{graphicx}
\graphicspath{{./images/}}
\newcommand{\@mainlogo}{SINTEF_Logo_Sentrert_\ifbool{print}{CMYK}{RGB}}

\RequirePackage{sinteflabels}
\InputIfFileExists{sintefconfig}{}{%
  \ClassWarning{sintefdoc}{%
  The sintefconfig.tex file is missing.\MessageBreak
  You can generate it by double-clicking sintefconfig.js., or by
  using the sintefconfig-example.tex as a template.}
}

% If some user has loaded hyperref,
% check for it and user the starred (non-linking) version of \pageref
% for page count (useless/ugly link)
\newcommand*\PlainPageRef[1]{%
  \@ifundefined{@pagerefstar}{\pageref{#1}}{\pageref*{#1}}%
}

% Information box
\newcommand{\@info}[1][sintefblue]{%
  \noindent%
  \scriptsize%
  \raggedright%
  \setlength{\parskip}{1ex}%
  \color{#1}%
  \textbf{\@institute}

  \@addresslabel:\\\@address

  \@phonelabel: \ifbool{english}{+47~}{}\@phone

  %\texttt{info@sintef.no}

  \@VATlabel:\\\@vat

}

% This separating line is used across several documents,
% for example in footers and title pages.
\RequirePackage{sintefcolor}
\newcommand{\@separator}{%
  % To make sure we have spacing on both sides, make an invisible rule, 2X tall
  \rule{0ex}{2ex}%
   % Place the dashed rule 1X high
  \textcolor{sintefgray}{\rule[1ex]{\textwidth}{0.25pt}}%
}
