import React, { useState } from "react";
import "./App.scss";
import Index from "./pages/index";
import { INDEX, ADMIN, QUERY, STAT } from "./constants/pages";
import Admin from "./pages/admin/admin";
import Query from "./pages/query/query";
import Stat from "./pages/stat/stat";

function App() {
  const [page, setPage] = useState(INDEX);
  return (
    <div className="App">
      {page !== INDEX && (
        <header className="header">
          <div className={`header-item ${page === INDEX ? "current" : ""}`} onClick={() => setPage(INDEX)}>
            主页
          </div>
          <div className={`header-item ${page === ADMIN ? "current" : ""}`} onClick={() => setPage(ADMIN)}>
            数据维护
          </div>
          <div className={`header-item ${page === QUERY ? "current" : ""}`} onClick={() => setPage(QUERY)}>
            数据查询
          </div>
          <div className={`header-item ${page === STAT ? "current" : ""}`} onClick={() => setPage(STAT)}>
            数据统计
          </div>
        </header>
      )}
      {page === INDEX && <Index setPage={setPage} />}
      <div className="content" hidden={page !== ADMIN}>
        <Admin />
      </div>
      <div className="content" hidden={page !== QUERY}>
        <Query />
      </div>
      <div className="content" hidden={page !== STAT}>
        <Stat />
      </div>
    </div>
  );
}

export default App;
