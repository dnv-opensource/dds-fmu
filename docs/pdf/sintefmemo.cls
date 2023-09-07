% Class for SINTEF memos with LaTeX.
%
% by Federico Zenith, federico.zenith@sintef.no.

\ProvidesClass{sintefmemo}[2022/01/04 A class to prepare memos in the SINTEF 
style]
\DeclareOption*{\PassOptionsToClass{\CurrentOption}{sintefdoc}}
\ProcessOptions\relax

\PassOptionsToPackage{table}{xcolor}
\LoadClass{sintefdoc}

% Set margins
\RequirePackage[top=4.75cm,        bottom=2.5cm,
                left=2.3cm,        right=2.3cm,
                headheight=4.25cm, headsep=5mm,
                footskip=2.5ex]{geometry}

\heading{\EnglishNorsk{Memo}{Notat}}
\renewcommand*{\@authorlabel}
              {\EnglishNorsk{Person Responsible / Author}
                            {Saksbehandler / Forfatter}}

% Setting up header and footer
\RequirePackage{fancyhdr}
\pagestyle{fancy}
\renewcommand{\headrulewidth}{0pt}
\fancyhf{}% clear all header and footer fields
\lhead{\renewcommand{\baselinestretch}{0}%
       \raisebox{4.85mm}{\includegraphics[height=3.62cm]{\@mainlogo}}%
       }

% Footer
\RequirePackage{lastpage}
\cfoot{%
  \color{sintefgray}
  \@separator\newline
  \hspace*{\tabcolsep}\hfill
  \thepage\ \EnglishNorsk{of}{av} \PlainPageRef{LastPage}%
  \hspace*{\tabcolsep}\newline
  \@separator
}

% Setting up header and footer for first page
\RequirePackage{lastpage}
\fancypagestyle{firstpage}[fancy]{%
  \rhead{\begin{minipage}[b]{0.25\textwidth}%
    \@info%
    \end{minipage}%
  }
}

\RequirePackage{xparse}
\newcommand*{\@distributiontable}{}
\newcommand*{\ifanyoffouris}[5]{ \ifboolexpr{
  test {\ifstrequal{#1}{#5}} or
  test {\ifstrequal{#2}{#5}} or
  test {\ifstrequal{#3}{#5}} or
  test {\ifstrequal{#4}{#5}}
} }
\DeclareDocumentCommand \recipient { >{\SplitArgument{3}{,}} O{} m }{
  \makeatletter
    \g@addto@macro \@distributiontable {
        \small #2 & \ifanyoffouris #1 {attention}  {\small$\bullet$}{}
                  & \ifanyoffouris #1 {comments}   {\small$\bullet$}{}
                  & \ifanyoffouris #1 {information}{\small$\bullet$}{}
                  & \ifanyoffouris #1 {agreed}     {\small$\bullet$}{}\\
    }
  \makeatother
}

% Recipient address and information colophon
\RequirePackage{colortbl,tabularx,setspace,rotating}

\newcommand{\frontmatter}{%
  \thispagestyle{firstpage}%
  \noindent%
  \begin{minipage}[b]{0.7\textwidth}
    \setlength{\parskip}{2ex}%
    \Huge \@heading

    \huge\textbf \@title
  \end{minipage}

  \vspace{4ex}%
  \noindent%
  \@separator\\
  \rowcolors{4}{}{sintefgray}
  \begin{tabularx}{\textwidth}{Xcccc}
  \rowcolor{white} \parbox{\linewidth}{{\@LabelText \@authorlabel}\\\@author}
    & \raisebox{-1.5cm}{\begin{sideways}%
        \parbox{2cm}{\raggedright\@LabelText\@attentionlabel}%
      \end{sideways}}
    & \raisebox{-1.5cm}{\begin{sideways}%
        \parbox{2cm}{\raggedright\@LabelText\@commentslabel}%
      \end{sideways}}
    & \raisebox{-1.5cm}{\begin{sideways}%
        \parbox{2cm}{\raggedright\@LabelText\@informationlabel}%
      \end{sideways}}
    & \raisebox{-1.5cm}{\begin{sideways}%
        \parbox{2cm}{\raggedright\@LabelText\@agreedlabel}%
      \end{sideways}}\\
    \rowcolor{white} \multicolumn{5}{@{}c@{}}{\@separator}\\
    \rowcolor{white} \@LabelText \@distributionlabel\\
    \@distributiontable
  \end{tabularx}
  \rowcolors{1}{}{} % Back to normal
  \@separator\\
  \hspace*{\tabcolsep}
  \begin{minipage}[t]{0.45\textwidth}
    \@LabelText \@projectlabel\\
    \@project
  \end{minipage}\hfill
  \begin{minipage}[t]{0.3\textwidth}
    \@LabelText \@datelabel\\
    \@date
  \end{minipage}
  \begin{minipage}[t]{0.2\textwidth}
    \@LabelText \@classificationlabel\\
    \@classification
  \end{minipage}\\
  \@separator

  \noindent
}
