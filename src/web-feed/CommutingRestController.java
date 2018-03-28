@Controller
@RequestMapping("/rest")
public class CommutingRestController {
	
	@ResponseBody
	@RequestMapping(value = "/commuting/tram-49", method = RequestMethod.GET)
	public String getNext49(HttpServletRequest request) {
		
		try {
			URL url = new URL("http://futar.bkk.hu/bkk-utvonaltervezo-api/ws/otp/api/where/plan-trip.json?showIntermediateStops=true&maxTransfers=5&fromCoord=47.470774%2C19.028865&fromName=Cs%C3%B3ka%20utca&toCoord=47.497677%2C19.054527&toName=De%C3%A1k%20Ferenc%20t%C3%A9r&fromPlace=Cs%C3%B3ka%20utca%3A%3A47.470774%2C19.028865&toPlace=De%C3%A1k%20Ferenc%20t%C3%A9r%3A%3A47.497677%2C19.054527&key=bkk-web&version=3&appVersion=2.3.5-20180116174736");
			HttpURLConnection con = (HttpURLConnection) url.openConnection();
			con.setRequestProperty("Content-Type", "application/json");
			con.setRequestMethod("GET");	
			
			BufferedReader in = new BufferedReader(new InputStreamReader(con.getInputStream()));
			String inputLine;
			StringBuffer content = new StringBuffer();
			while ((inputLine = in.readLine()) != null) {
			    content.append(inputLine);
			}
			in.close();
			
			ObjectMapper mapper = new ObjectMapper();
			
			JsonNode rootNode = mapper.readTree(content.toString());
			JsonNode nodes = rootNode.get("data").get("entry").get("plan").get("itineraries");
			String startTime = null;
			Long candidate = null;
			Long bestCandidate = null;
			for (int i=0; i!=nodes.size(); i++) {
				
				JsonNode plan = nodes.get(i);
				System.out.println("Plan [" + i + "] leg size: " + plan.get("legs").size());
				
				for (int j=0; j!=plan.get("legs").size(); j++) {
					String mode = plan.get("legs").get(j).get("mode").toString().replaceAll("\"", "");
					String route = plan.get("legs").get(j).get("route").toString().replaceAll("\"", "");
					
					System.out.println("  -> Legs["+j+"].mode = " + mode);
					System.out.println("  -> Legs["+j+"].route = " + route);
					
					if (route.equals("49")) {
						startTime = plan.get("startTime").toString();
						candidate = Long.parseLong(startTime);
						System.out.println("Candidate await time in minutes: " + this.getAwaitMinutes(candidate));
						if (bestCandidate == null) {
							bestCandidate = candidate;
						}
						else {
							if (candidate < bestCandidate) {
								bestCandidate = candidate;
								System.out.println("Best candidate so far for route 49: " + startTime);
							}
						}
					}
				}
			}
			
			if (startTime == null) {
				throw new Exception("Route 49 not found");
			}
			
			Date nextTram = new Date(bestCandidate);
			
			Long currentTime = Calendar.getInstance().getTime().getTime();
			Long nextTramTime = nextTram.getTime();
			
			Long nextTramInMinutes = (nextTramTime - currentTime) / 1000;
			return nextTramInMinutes.toString();
			
		}
		catch (Exception error) {
			error.printStackTrace();
			return "e!";
		}
	
	}
}
