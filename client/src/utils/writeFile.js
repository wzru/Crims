import * as XLSX from "xlsx";
import HEADER_MAP from "../constants/headerMap";

export default function writeWorkbookToLocalFile(jsonArr, sheetName) {
  let newjsonArr = jsonArr.map((oldJson) => {
    let newJson = {};
    for (let key in oldJson) {
      let newKey = key in HEADER_MAP ? HEADER_MAP[key] : key;
      newJson[newKey] = oldJson[key];
    }
    console.log(newJson);
    return newJson;
  });
  const workSheet = XLSX.utils.json_to_sheet(newjsonArr);
  const workBook = XLSX.utils.book_new();
  XLSX.utils.book_append_sheet(workBook, workSheet, sheetName);
  XLSX.writeFile(workBook, `${sheetName}-导出.xlsx`);
}
