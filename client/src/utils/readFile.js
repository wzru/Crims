import * as XLSX from "xlsx";
export default function readWorkbookFromLocalFile(file) {
  return new Promise((resolve, reject) => {
    const reader = new FileReader();
    reader.onload = function (e) {
      const { result } = e.target;
      const workbook = XLSX.read(result, { type: "binary" });
      let jsonData = [];
      for (const sheet in workbook.Sheets) {
        jsonData = jsonData.concat(XLSX.utils.sheet_to_json(workbook.Sheets[sheet]));
      }
      resolve(jsonData);
    };
    reader.readAsBinaryString(file);
  });
}
