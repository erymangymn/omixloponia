import OpenAI from "openai";

const openai = new OpenAI({
  apiKey: "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",
});

const response = openai.responses.create({
  model: "gpt-5-nano",
  input: "fogponic frequency time on just number",
  store: true,
});

response.then((result) => console.log(result.output_text));