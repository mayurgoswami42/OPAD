import html
import json

class ReportBuilder:
    def __init__(self):
        self.__REPORT_ID: int = -1

    def __esc(self, value: str) -> str:
        return html.escape(str(value), quote=True)

    def __get_report_id(self) -> int:
        if self.__REPORT_ID == -1:
            with open("storage/.report_id", "r+") as file:
                self.__REPORT_ID = int(file.readline().strip())
                file.seek(0)
                file.truncate()
                file.write(str(self.__REPORT_ID+1))
        self.__REPORT_ID += 1
        return self.__REPORT_ID

    def __update_home(self, json_object, url: str) -> None:
        inserter: str = "<div class=\"inserter\">$INSERTER$</div>"
        
        with open("templates/report_card.html", "r") as report_file:
            report_card = report_file.read()
            report_card = report_card.replace("$ANOMALY_TYPE$", self.__esc(json_object["anomaly_type"]))
            report_card = report_card.replace("$DATE$", self.__esc(json_object["date"]))
            report_card = report_card.replace("$TIME$", self.__esc(json_object["time"]))
            report_card = report_card.replace("$REPORT_LINK$", url)

        with open("frontend/index.html", "r+") as file:
            page: str = file.read()

            page = page.replace(inserter, report_card + inserter)

            file.seek(0)
            file.truncate()
            file.write(page)

    def save_reports(self, data: str) -> None:
        try:
            json_object = json.loads(data)
        except json.JSONDecodeError as e:
            print(f"[report_builder] discarding malformed report: {e}")
            return

        id = self.__get_report_id()
        with open(f"frontend/reports/report_{id}.html", "w") as file:
            page: str = open("templates/report.html", "r").read()
            page = page.replace("$DATE$", self.__esc(json_object["date"]))
            page = page.replace("$TIME$", self.__esc(json_object["time"]))
            page = page.replace("$REPORTED_DATE$", self.__esc(json_object["reported_date"]))
            page = page.replace("$REPORTED_TIME$", self.__esc(json_object["reported_time"]))
            page = page.replace("$CLIENT_IP$", self.__esc(json_object["user_ip"]))
            page = page.replace("$REQUEST_METHOD$", self.__esc(json_object["method"]))
            page = page.replace("$PROTOCOL$", self.__esc(json_object["protocol"]))
            page = page.replace("$VERSION$", self.__esc(json_object["protocol_version"]))
            page = page.replace("$ANOMALY_TYPE$", self.__esc(json_object["anomaly_type"]))
            page = page.replace("$ANOMALY_MESSAGE$", self.__esc(json_object["anomaly_message"]))
            file.write(page)

        self.__update_home(json_object, f"report_{id}.html")