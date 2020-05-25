const debruijn = (k, n) => {
  const a = [...Array(k * n)].fill(0);
  let sequence = [];

  const db = (t, p) => {
    if (t > n) {
      if (n % p === 0) {
        sequence = sequence.concat(a.slice(1, p + 1));
      }
    } else {
      a[t] = a[t - p];
      db(t + 1, p);
      for (let j = a[t - p] + 1; j <= k; j++) {
        a[t] = j;
        db(t + 1, t);
      }
    }
  };

  db(1, 1);
  return sequence.join('');
};

const binaryStreamStringToBytes = (string) => {
  // 1100110001011001 -> 11001100 01011001
  let chunks = string.split(/(.{8})/).filter((x) => x);
  // 11001100 01011001 -> 00110011 10011010
  // chunks = chunks.map((x) => [...x].reverse().join(''));
  // 00110011 10011010 -> 0x33 0x9a
  chunks = chunks.map((x) =>
    `0x${parseInt(x, 2).toString(16).padStart(2, 0)}`);

  return chunks.join(', ');
};

const printChunks = (byteString) =>
  byteString.split(' ').reduce((s, x, i) => {
    const p = Math.floor(i / 9);
    s[p] = (s[p] || []).concat(x);
    return s;
  }, []).forEach((x) => console.log(x.join(' ')));

module.exports = {
  binaryStreamStringToBytes,
  debruijn,
  printChunks,
};

if (require.main === module) {
  // de bruijn for hekapoo.h
  console.log(printChunks(
    binaryStreamStringToBytes(
      debruijn(1, 10))));
}
