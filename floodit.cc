#include <cassert>
#include <bitset>
#include <queue>
#include <set>
#include <string>
using namespace std;

#define FOR(i,n) for(int i=0;i<n;++i)

const int MAXR=12, MAXC=12;
const int NBITS=MAXR*MAXC;

int toidx(int r, int c) {
  return r*MAXC + c;
}

const int NCOLOURS = 6;
int fromcolour(char ch) {
  switch (ch) {
    case '.': return -1;
    case 'B': return 0;
    case 'G': return 1;
    case 'M': return 2;
    case 'O': return 3;
    case 'R': return 4;
    case 'Y': return 5;
  }
  assert(false);
  return -2;
}

char tocolour(int x) {
  switch (x) {
    case -1: return '.';
    case 0: return 'B';
    case 1: return 'G';
    case 2: return 'M';
    case 3: return 'O';
    case 4: return 'R';
    case 5: return 'Y';
  }
  assert(false);
  return 'x';
}

int R, C;
int board[MAXR][MAXC];

const int D = 4;
int dr[] = { 0, 1, 0, -1 };
int dc[] = { 1, 0, -1, 0 };

const int inf = 123456789;
int depth[MAXR][MAXC];
void compute_depth(const bitset<NBITS> &dead) {
  bool mark[MAXR][MAXC];
  memset(mark, 0, sizeof(mark));

  FOR(r,R) FOR(c,C) depth[r][c] = inf;

  deque<pair<int,int> > q;
  q.push_back(make_pair(0,0));
  depth[0][0] = 0;
  while (q.size()) {
    int r = q.front().first, c = q.front().second;
    q.pop_front();

    if (mark[r][c]) continue;
    mark[r][c] = 1;

    FOR(i,D) {
      int r2 = r+dr[i], c2 = c+dc[i];
      if (r2<0 || r2>=R || c2<0 || c2>=C) continue;
      
      int d = depth[r][c];
      int incr = !dead[toidx(r2,c2)] && board[r2][c2] != board[r][c];
      d += incr;

      if (d < depth[r2][c2]) {
        depth[r2][c2] = d;

        if (incr) q.push_back(make_pair(r2,c2));
        else q.push_front(make_pair(r2,c2));
      }
    }
  }
}

int heuristic_value(const bitset<NBITS> &dead) {
  int maxdepth[NCOLOURS] = {};

  FOR(r,R) FOR(c,C) if (!dead[toidx(r,c)]) {
    int u = board[r][c];
    maxdepth[u] = max(maxdepth[u], depth[r][c]);
  }

  int m = 0;
  FOR(u,NCOLOURS) m = max(m, maxdepth[u]);

  int ans = 0;
  for (int d = 1; d <= m; ++d) {
    int c = 0;
    FOR(u,NCOLOURS) if (maxdepth[u] >= d) ++c;
    ans = max(ans, c+d-1);
  }
  return ans;
}

void dump_depth() {
  printf("\n");
  FOR(r,R) {
    printf("[ ");
    FOR(c,C) {
      printf("%2d ", depth[r][c]);
    }
    printf("]\n");
  }
}

struct state {
  int moves;
  string path;
  bitset<NBITS> dead;
};

struct ltbitset {
  bool operator()(const bitset<NBITS> &s, const bitset<NBITS> &t) const {
    // This is seriously ugly.
    if (s == t) return false;
    if (s.count() != t.count()) return s.count() < t.count();
    FOR(i,NBITS) {
      if (s[i] != t[i]) return s[i] < t[i];
    }
    return false;
  }
};

int main() {
  scanf(" %d%d",&R,&C);

  bitset<NBITS> wanted;
  FOR(r,R) {
    FOR(c,C) {
      char ch;
      scanf(" %c", &ch);
      board[r][c] = fromcolour(ch);

      wanted[toidx(r,c)] = 1;
    }
  }

  state init;
  init.moves = 0;
  init.dead[toidx(0,0)] = 1;

  int prevmoves = 0;
  int heurdistr[32] = {}; // FIXME

  set<bitset<NBITS>, ltbitset> mark;
  deque<state> q;
  q.push_back(init);

  while (q.size()) {
    state cur = q.front();
    q.pop_front();

    if (mark.count(cur.dead)) continue;
    mark.insert(cur.dead);

    if (cur.moves > prevmoves) {
      prevmoves = cur.moves;
      printf("  Moves = %2d, |q| = %d\n", cur.moves, (int)q.size());
      FOR(h,32) if (heurdistr[h]) {
	printf("    Heuristic value %2d:  %d\n", h, heurdistr[h]);
      }

      memset(heurdistr, 0, sizeof(heurdistr));
    }

    if (cur.dead == wanted) {
      printf("%d\n", cur.moves);
      string path = cur.path;
      FOR(i,(int)path.size()) {
        printf("%c", path[i]);
      }
      printf("\n");
      return 0;
    }

    compute_depth(cur.dead);
    int heur = heuristic_value(cur.dead);
    ++heurdistr[heur];

    FOR(u,NCOLOURS) {
      state next = cur;
      ++next.moves;
      next.path.push_back(tocolour(u));

      FOR(r,R) FOR(c,C) if (depth[r][c] == 1 && board[r][c] == u) {
        next.dead[toidx(r,c)] = 1;
      }

      q.push_back(next);
    }
  }

  printf("No solution found.\n");
}
