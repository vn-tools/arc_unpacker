# rubocop:disable Style/AsciiComments

# How to find out the key for your game:
# 1. Search for _wsprintfA("%s%d", ...) instruction.
# 2. The register used to fill "%s" part should point to the key.
# 3. Apparently, this is just the main window title, encoded in SJIS.

MSD_KEYS = {
  sonohana1: 'その花びらにくちづけを'.encode('sjis'),
  sonohana2: 'その花びらにくちづけを　わたしの王子さま'.encode('sjis'),
  sonohana3: 'その花びらにくちづけを　あなたと恋人つなぎ'.encode('sjis'),
  sonohana4: 'その花びらにくちづけを　愛しさのフォトグラフ'.encode('sjis'),
  sonohana5: 'その花びらにくちづけを　あなたを好きな幸せ'.encode('sjis'),
  sonohana6: 'その花びらにくちづけを　唇とキスで呟いて'.encode('sjis'),
  sonohana7: 'その花びらにくちづけを　あまくてほしくてとろけるちゅう'.encode('sjis'),
  sonohana8: 'その花びらにくちづけを　天使の花びら染め'.encode('sjis'),
  sonohana9: 'その花びらにくちづけを　あまくておとなのとろけるちゅう'.encode('sjis'),
  sonohana10: 'その花びらにくちづけを　リリ・プラチナム'.encode('sjis'),
  sonohana11: 'その花びらにくちづけを ミカエルの乙女たち'.encode('sjis')
}
